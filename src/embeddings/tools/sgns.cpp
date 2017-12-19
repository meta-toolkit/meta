/**
 * @file sgns.cpp
 * @author Shawn LeMaster
 * @author Matt Kelly
 *
 * Builds word embedding vectors using the Skip-Gram Negative Sampling method.
 *
 * This code is heavily based on the original word2vec implementation
 * from Tomas Mikolov et. al. but has been adapted to use C++ features
 * where it increases speed or maintainability.
 *
 * @see https://code.google.com/archive/p/word2vec/
 * @see https://github.com/svn2github/word2vec
 *
 * The original code was licensed under the Apache 2.0 license which may be
 * found
 * here: http://www.apache.org/licenses/LICENSE-2.0
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#include <atomic>
#include <chrono>
#include <deque>
#include <iostream>
#include <memory>
#include <random>

#include "cpptoml.h"
#include "meta/analyzers/all.h"
#include "meta/analyzers/token_stream.h"
#include "meta/corpus/corpus_factory.h"
#include "meta/hashing/probe_map.h"
#include "meta/io/filesystem.h"
#include "meta/io/packed.h"
#include "meta/logging/logger.h"
#include "meta/parallel/thread_pool.h"
#include "meta/util/aligned_allocator.h"
#include "meta/util/printing.h"
#include "meta/util/progress.h"
#include "meta/util/random.h"

using namespace meta;

using sgns_net_vector = util::aligned_vector<float, 128>;

class sgns_local_buffer
{
  public:
    sgns_local_buffer(const analyzers::token_stream& stream,
                      const std::size_t vector_size)
        : stream_{stream.clone()},
          neu1e(vector_size, 0),
          engine(static_cast<uint64_t>(
              std::chrono::system_clock::now().time_since_epoch().count())),
          next_real{0.0f, 1.0f},
          word_counter(0)
    {
    }

    sgns_local_buffer(sgns_local_buffer&&) = default;

  private:
    friend class sgns_trainer;
    std::unique_ptr<analyzers::token_stream> stream_;
    sgns_net_vector neu1e;
    std::default_random_engine engine;
    std::uniform_real_distribution<float> next_real;
    uint64_t word_counter;
};

class sgns_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

// Represents a word in the vocabulary.
struct sgns_word
{
    std::string word;
    uint64_t count;
};

using sgns_vocab_vector = std::vector<sgns_word>;
using sgns_vocab_index_map
    = hashing::probe_map<std::string, sgns_vocab_vector::size_type>;

// The vocabulary is represented by a vector of sgns_word structures and a table
// that maps from word strings to their vector indexes.
struct sgns_vocab
{
    sgns_vocab_vector vector;
    hashing::probe_map<std::string, sgns_vocab_vector::size_type> table;
    uint64_t total_count;
};

using sgns_noise_distribution = std::vector<std::size_t>;
using sgns_window = std::deque<sgns_vocab_vector::size_type>;

// Represents the neural network that is trained.
struct sgns_net
{
    sgns_net(std::size_t vocab_size, std::size_t layer1_size)
        : syn0(vocab_size * layer1_size), syn1neg(vocab_size * layer1_size, 0)
    {
        auto seed = static_cast<uint64_t>(
            std::chrono::system_clock::now().time_since_epoch().count());
        std::default_random_engine random(seed);
        std::uniform_real_distribution<float> uniform(
            -0.5f / (float)layer1_size, 0.5f / (float)layer1_size);

        for (auto& i : syn0)
            i = uniform(random);
    }

    sgns_net_vector syn0;
    sgns_net_vector syn1neg;
};

// The primary class for executing SGNS.
class sgns_trainer
{
  public:
    // Constructor.
    sgns_trainer(const cpptoml::table& cfg, const cpptoml::table& embed_cfg,
                 const cpptoml::table& sgns_cfg)
        : cfg_(cfg),
          embed_cfg_(embed_cfg),
          prefix_(*embed_cfg.get_as<std::string>("prefix")),
          vector_size_(
              embed_cfg.get_as<std::size_t>("vector-size").value_or(100)),
          num_threads_(
              embed_cfg.get_as<std::size_t>("num-threads")
                  .value_or(std::max(1u, std::thread::hardware_concurrency()))),
          max_ram_(embed_cfg.get_as<std::size_t>("max-ram").value_or(4096)
                   * 1024 * 1024),
          subsample_threshold_(
              (float)sgns_cfg.get_as<double>("subsample-threshold")
                  .value_or(1e-4)),
          max_window_size_(
              sgns_cfg.get_as<std::size_t>("max-window-size").value_or(6)),
          iterations_(sgns_cfg.get_as<uint64_t>("iterations").value_or(10)),
          starting_learning_rate_(
              (float)sgns_cfg.get_as<double>("learning-rate").value_or(0.025)),
          negative_samples_(
              sgns_cfg.get_as<std::size_t>("negative-samples").value_or(20)),
          vocab_(load_vocab(prefix_ + "/vocab.bin")),
          noise_dist_(create_unigram_noise_distribution(
              sgns_cfg.get_as<std::size_t>("unigram-distribution-size")
                  .value_or(1E8),
              sgns_cfg.get_as<double>("unigram-distribution-power")
                  .value_or(0.75))),
          net_(vocab_.vector.size(), vector_size_),
          learning_rate_(starting_learning_rate_),
          word_count_actual_(0)
    {
        {
            std::mutex io_mutex;
            printing::progress progress{
                " > Training: ", (uint64_t)(vocab_.total_count * iterations_)};

            train(io_mutex, progress);
        }

        save_meta_vectors();
    }

    // When the thread_pool goes out of scope, the threads will join. In other
    // words, when this method returns, all threads have finished training.
    void train(std::mutex& io_mutex, printing::progress& progress)
    {
        auto stream = analyzers::load_filters(cfg_, embed_cfg_);
        if (!stream)
        {
            throw sgns_exception{
                "Failed to find an ngram-word analyzer configuration"};
        }

        parallel::thread_pool pool{num_threads_};

        auto iterations_left = iterations_;

        while (iterations_left > 0)
        {
            auto docs = corpus::make_corpus(cfg_);

            corpus::parallel_consume(
                *docs, pool,
                [&]() {
                    return sgns_local_buffer{*stream, vector_size_};
                },
                [&](sgns_local_buffer& buffer, const corpus::document& doc) {
                    process_document(buffer, doc, io_mutex, progress);
                });

            --iterations_left;
        }
    }

    // Process a document
    void process_document(sgns_local_buffer& buffer,
                          const corpus::document& doc, std::mutex& io_mutex,
                          printing::progress& progress)
    {
        buffer.stream_->set_content(analyzers::get_content(doc));

        // Holds the target word and surrounding context words.
        sgns_window window;

        // Load target word.
        add_next_index(window, buffer);

        // Initialize future context words.
        while (window.size() < (max_window_size_ + 1))
            if (!add_next_index(window, buffer))
                break;

        // When the window is first loaded, the target word is the
        // first element in the window. The next time a word is added
        // to the window, this index will increase by one. This will
        // continue until this index reaches the middle of the window,
        // where it will stay until the algorithm completes.
        sgns_window::size_type target_widx = 0;

        // While there are target words left in the window...
        while (!window.empty() && (target_widx < window.size()))
        {
            // Sample a random window size.
            const auto window_size
                = random::bounded_rand(buffer.engine, max_window_size_);

            // Sweep across the window.
            for (sgns_window::size_type w = window_size;
                 w < (max_window_size_ * 2 + 1 - window_size); ++w)
            {
                // Don't process a target word as its own context word.
                // Max window size works here because it is window size + 1th
                // index
                // and therefore the target word
                if (w != max_window_size_)
                {
                    const int64_t context_widx = (int64_t)target_widx
                                                 - (int64_t)max_window_size_
                                                 + (int64_t)w;

                    if (context_widx < 0
                        || context_widx >= (int64_t)window.size())
                        continue;

                    const sgns_net_vector::size_type l1
                        = window[(sgns_window::size_type)context_widx]
                          * vector_size_;

                    buffer.neu1e.assign(vector_size_, 0);

                    // The first sample here will use the real
                    // target. The rest will be negative samples.
                    update_vectors(1, window[target_widx], buffer.neu1e, l1);
                    for (std::size_t n = 0; n < negative_samples_; n++)
                    {
                        sgns_net_vector::size_type target
                            = noise_dist_[random::bounded_rand(
                                buffer.engine, noise_dist_.size())];
                        if (target == window[target_widx])
                            continue;
                        update_vectors(0, target, buffer.neu1e, l1);
                    }

                    for (sgns_net_vector::size_type i = 0; i < vector_size_;
                         i++)
                    {
                        net_.syn0[l1 + i] += buffer.neu1e[i];
                    }
                }
            }

            // Load the next word in the document into the window.
            if (add_next_index(window, buffer))
            {
                ++target_widx;

                // If the window is now too big, drop the first element.
                if (window.size() > (max_window_size_ * 2 + 1))
                {
                    window.pop_front();
                    --target_widx;
                }
            }
            else
            {
                // If the document is out of words, continue dropping
                // elements off the front of the window.
                window.pop_front();
            }

            // Update our learning rate every 10,000 words read.
            if (buffer.word_counter >= 10000)
            {
                update_progress(buffer.word_counter, io_mutex, progress);
                buffer.word_counter = 0;
            }
        }
    }

    // Update the vectors
    void update_vectors(int label, sgns_net_vector::size_type& target,
                        sgns_net_vector& neu1e,
                        const sgns_net_vector::size_type& l1)
    {
        const sgns_net_vector::size_type l2 = target * vector_size_;

        float dot_product = 0.0f;
        for (sgns_net_vector::size_type i = 0; i < vector_size_; i++)
        {
            dot_product += net_.syn0[l1 + i] * net_.syn1neg[l2 + i];
        }

        const float sigma = 1.0f / (1.0f + (float)std::exp(-dot_product));
        const float update = learning_rate_ * (label - sigma);

        for (sgns_net_vector::size_type i = 0; i < vector_size_; i++)
        {
            neu1e[i] += update * net_.syn1neg[l2 + i];
            net_.syn1neg[l2 + i] += update * net_.syn0[l1 + i];
        }
    }

    // Update the progress indicator
    void update_progress(uint64_t word_counter, std::mutex& io_mutex,
                         printing::progress& progress)
    {
        // Note: This learning rate update is subject to a race condition
        // when running multiple threads however this race condition
        // also exists in the word2vec and doesn't appear to
        // have any impact.
        word_count_actual_ += word_counter;

        learning_rate_ = std::max(
            starting_learning_rate_
                * (1.0f
                   - (float)word_count_actual_
                         / (float)(iterations_ * vocab_.total_count + 1.0f)),
            starting_learning_rate_ * 0.0001f);

        {
            std::lock_guard<std::mutex> lock{io_mutex};
            progress(word_count_actual_);
        }
    }

    // Grabs the next word from the document stream, performs subsampling,
    // and adds the vocabulary index for the word to the context window.
    bool add_next_index(sgns_window& window, sgns_local_buffer& buffer)
    {
        while (*buffer.stream_)
        {
            const std::string word = buffer.stream_->next();
            auto i = vocab_.table.find(word);

            // Ignore out-of-vocabulary words.
            if (i != vocab_.table.end())
            {
                ++buffer.word_counter;

                if (subsample_threshold_ > 0)
                {
                    // Subsampling which randomly discards frequent words but
                    // does not change the rankings
                    const auto count = vocab_.vector[i->value()].count;
                    const float subsample_count_
                        = subsample_threshold_ * vocab_.total_count;
                    const float ran = (std::sqrt(count / subsample_count_) + 1)
                                      * subsample_count_ / count;
                    if (ran < (buffer.next_real(buffer.engine)))
                    {
                        continue;
                    }
                }

                window.push_back(i->value());

                return true;
            }
        }

        return false;
    }

    // Load the vocabulary from a file.
    sgns_vocab load_vocab(const std::string& filename) const
    {
        if (!filesystem::file_exists(filename))
        {
            LOG(fatal) << "Vocabulary has not yet been generated, please "
                          "do this before learning word embeddings"
                       << ENDLG;
            throw sgns_exception{"no vocabulary file found in " + prefix_};
        }

        std::ifstream input{filename, std::ios::binary};
        const auto size = io::packed::read<uint64_t>(input);

        sgns_vocab vocab{};

        printing::progress progress{" > Loading vocab: ", size};

        for (uint64_t tid{0}; tid < size; ++tid)
        {
            progress(tid);
            const auto word = io::packed::read<std::string>(input);
            const auto count = io::packed::read<uint64_t>(input);
            vocab.table[word] = tid;
            vocab.vector.push_back(sgns_word{word, count});
            vocab.total_count += count;
        }

        progress.end();

        const uint64_t bytes_used
            = (vocab.vector.size() * sizeof(sgns_vocab_vector::value_type))
              + vocab.table.bytes_used();

        LOG(info) << "Loaded vocabulary of size " << vocab.table.size()
                  << " occupying " << printing::bytes_to_units(bytes_used)
                  << ENDLG;

        if ((max_ram_ <= bytes_used)
            || ((max_ram_ - bytes_used) < (1024 * 1024)))
        {
            throw sgns_exception{"RAM limit too restrictive"};
        }

        return vocab;
    }

    // Create a vector whose elements are present in proportion
    // to their noise sample probablility
    sgns_noise_distribution
    create_unigram_noise_distribution(sgns_noise_distribution::size_type size,
                                      double power) const
    {
        printing::progress progress{" > Generating noise distribution: ",
                                    vocab_.vector.size()};

        sgns_noise_distribution noise_dist(size);

        double normalizer = 0;
        for (sgns_vocab_vector::size_type i = 0; i < vocab_.vector.size(); ++i)
            normalizer += std::pow(vocab_.vector[i].count, power);

        sgns_noise_distribution::size_type i = 0;
        double cumulative_probability = 0;

        for (sgns_vocab_vector::size_type j = 0; j < vocab_.vector.size(); ++j)
        {
            progress(j);
            cumulative_probability
                += std::pow(vocab_.vector[j].count, power) / normalizer;

            while ((i < noise_dist.size())
                   && ((float)i / noise_dist.size() < cumulative_probability))
            {
                noise_dist[i] = j;
                ++i;
            }
        }

        progress.end();

        LOG(info) << "Created smoothed unigram noise distribution of size "
                  << noise_dist.size() << ENDLG;

        return noise_dist;
    }

    // Save vectors in MeTA format.
    void save_meta_vectors() const
    {
        // Target embeddings.
        {
            std::ofstream output{prefix_ + "/embeddings.target.bin",
                                 std::ios::binary};
            printing::progress progress{" > Saving target embeddings: ",
                                        net_.syn0.size()
                                            + vocab_.vector.size()};
            uint64_t words_processed = 0;

            io::packed::write(output, vector_size_);

            // Save target vectors.
            for (auto i : net_.syn0)
            {
                io::packed::write(output, (double)i);
                progress(++words_processed);
            }

            // Write out the unk vector.
            for (std::size_t i = 0; i < vector_size_; ++i)
            {
                io::packed::write(output, 0.0);
                progress(++words_processed);
            }
        }

        // Context embeddings.
        {
            std::ofstream output{prefix_ + "/embeddings.context.bin",
                                 std::ios::binary};
            printing::progress progress{" > Saving context embeddings: ",
                                        net_.syn1neg.size()
                                            + vocab_.vector.size()};
            uint64_t words_processed = 0;

            io::packed::write(output, vector_size_);

            // Save context vectors.
            for (auto i : net_.syn1neg)
            {
                io::packed::write(output, (double)i);
                progress(++words_processed);
            }

            // Write out the unk vector.
            for (std::size_t i = 0; i < vector_size_; ++i)
            {
                io::packed::write(output, 0.0);
                progress(++words_processed);
            }
        }
    }

  private:
    // References to tables in config.toml.
    const cpptoml::table& cfg_;
    const cpptoml::table& embed_cfg_;

    // Embedding parameters from config.toml.
    const std::string prefix_;
    const std::size_t vector_size_;
    const std::size_t num_threads_;
    const std::size_t max_ram_;

    // SGNS parameters from config.toml.
    const float subsample_threshold_; // Sample in word2vec
    const std::size_t max_window_size_;
    const uint64_t iterations_;
    const float starting_learning_rate_; // Alpha in word2vec
    const std::size_t negative_samples_;

    // Immutable shared data.
    const sgns_vocab vocab_;
    const sgns_noise_distribution noise_dist_;

    // Mutable shared data.
    sgns_net net_;
    float learning_rate_;
    std::atomic<uint64_t> word_count_actual_;
};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto cfg = cpptoml::parse_file(argv[1]);

    auto embed_cfg = cfg->get_table("embeddings");
    if (!embed_cfg)
    {
        std::cerr << "Missing [embeddings] configuration in " << argv[1]
                  << std::endl;
        return 1;
    }

    auto sgns_cfg = embed_cfg->get_table("sgns");
    if (!sgns_cfg)
    {
        std::cerr << "Missing [embeddings.sgns] configuration in " << argv[1]
                  << std::endl;
        return 1;
    }

    try
    {
        sgns_trainer trainer{*cfg, *embed_cfg, *sgns_cfg};
    }
    catch (const sgns_exception& ex)
    {
        LOG(fatal) << ex.what() << ENDLG;
        return 1;
    }

    return 0;
}
