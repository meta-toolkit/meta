/**
 * @file glove.cpp
 * @author Chase Geigle
 *
 * This tool builds word embedding vectors from a weighted coocurrence
 * matrix using the GloVe model.
 *
 * @see http://nlp.stanford.edu/projects/glove/
 */

#include <thread>

#include "cpptoml.h"
#include "meta/embeddings/coocur_iterator.h"
#include "meta/io/filesystem.h"
#include "meta/io/packed.h"
#include "meta/logging/logger.h"
#include "meta/parallel/thread_pool.h"
#include "meta/util/aligned_allocator.h"
#include "meta/util/array_view.h"
#include "meta/util/progress.h"
#include "meta/util/printing.h"
#include "meta/util/random.h"
#include "meta/util/time.h"

using namespace meta;

std::size_t shuffle_partition(const std::string& prefix, std::size_t max_ram,
                              std::size_t num_partitions)
{
    using namespace embeddings;

    using vec_type = std::vector<coocur_record>;
    using diff_type = vec_type::iterator::difference_type;

    std::mt19937 engine{std::random_device{}()};
    vec_type records(max_ram / sizeof(coocur_record));

    // read in RAM sized chunks and shuffle in memory and write out to disk
    std::vector<uint64_t> chunk_sizes;

    std::size_t total_records = 0;
    coocur_iterator input{prefix + "/coocur.bin"};

    auto elapsed = common::time(
        [&]()
        {
            printing::progress progress{" > Shuffling (pass 1): ",
                                        input.total_bytes()};
            while (input != coocur_iterator{})
            {
                std::size_t i = 0;
                for (; i < records.size() && input != coocur_iterator{};
                     ++i, ++input)
                {
                    progress(input.bytes_read());
                    records[i] = *input;
                }

                std::shuffle(records.begin(),
                             records.begin() + static_cast<diff_type>(i),
                             engine);

                std::ofstream output{prefix + "/coocur-shuf."
                                         + std::to_string(chunk_sizes.size())
                                         + ".tmp",
                                     std::ios::binary};

                total_records += i;
                chunk_sizes.push_back(i);
                for (std::size_t j = 0; j < i; ++j)
                    io::packed::write(output, records[j]);
            }
        });

    LOG(info) << "Shuffling pass 1 took " << elapsed.count() / 1000.0
              << " seconds" << ENDLG;

    std::vector<coocur_iterator> chunks;
    chunks.reserve(chunk_sizes.size());
    for (std::size_t i = 0; i < chunk_sizes.size(); ++i)
    {
        chunks.emplace_back(prefix + "/coocur-shuf." + std::to_string(i)
                            + ".tmp");
    }

    std::vector<std::ofstream> outputs(num_partitions);
    for (std::size_t i = 0; i < outputs.size(); ++i)
    {
        outputs[i].open(prefix + "/coocur-shuf." + std::to_string(i) + ".bin",
                        std::ios::binary);
    }

    {
        printing::progress progress{" > Shuffling (pass 2): ", total_records};
        uint64_t num_read = 0;
        while (true)
        {
            // read in records from each chunk on disk. Records are taken from
            // chunks with probability proportional to their total size (in
            // records).
            std::size_t i = 0;
            for (std::size_t j = 0; j < chunk_sizes.size(); ++j)
            {
                auto to_write = std::max<std::size_t>(
                    static_cast<std::size_t>(static_cast<double>(chunk_sizes[j])
                                             / total_records * records.size()),
                    1);

                for (std::size_t n = 0; n < to_write; ++n)
                {
                    if (chunks[j] == coocur_iterator{} || i == records.size())
                        break;
                    records[i] = *chunks[j];
                    ++chunks[j];
                    ++i;
                    progress(++num_read);
                }
            }

            if (i == 0)
                break;

            // partition the records into the output files randomly
            for (std::size_t j = 0; j < i; ++j)
            {
                auto idx = random::bounded_rand(engine, outputs.size());
                io::packed::write(outputs[idx], records[j]);
            }
        }
    }

    // delete temporary files
    for (std::size_t i = 0; i < chunk_sizes.size(); ++i)
    {
        filesystem::delete_file(prefix + "/coocur-shuf." + std::to_string(i)
                                + ".tmp");
    }

    return total_records;
}

class glove_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

class glove_trainer
{
  public:
    glove_trainer(const cpptoml::table& embed_cfg)
    {
        // extract building parameters
        auto prefix = *embed_cfg.get_as<std::string>("prefix");
        auto max_ram = static_cast<std::size_t>(
                           embed_cfg.get_as<int64_t>("max-ram").value_or(4096))
                       * 1024 * 1024;
        vector_size_ = static_cast<std::size_t>(
            embed_cfg.get_as<int64_t>("vector-size").value_or(50));

        auto num_threads = static_cast<std::size_t>(
            embed_cfg.get_as<int64_t>("num-threads")
                .value_or(std::max(1u, std::thread::hardware_concurrency())));

        auto iters = static_cast<std::size_t>(
            embed_cfg.get_as<int64_t>("max-iter").value_or(25));

        learning_rate_
            = embed_cfg.get_as<double>("learning-rate").value_or(0.05);
        xmax_ = embed_cfg.get_as<double>("xmax").value_or(100.0);
        scale_ = embed_cfg.get_as<double>("scale").value_or(0.75);

        auto num_rare = static_cast<uint64_t>(
            embed_cfg.get_as<int64_t>("unk-num-avg").value_or(100));

        if (!filesystem::file_exists(prefix + "/vocab.bin"))
        {
            LOG(fatal) << "Vocabulary has not yet been generated, please "
                          "do this before learning word embeddings"
                       << ENDLG;
            throw glove_exception{"no vocabulary file found in " + prefix};
        }

        if (!filesystem::file_exists(prefix + "/coocur.bin"))
        {
            LOG(fatal)
                << "Coocurrence matrix has not yet been generated, please "
                   "do this before learning word embeddings"
                << ENDLG;
            throw glove_exception{"no coocurrence matrix found in " + prefix};
        }

        std::size_t num_words = 0;
        {
            std::ifstream vocab{prefix + "/vocab.bin", std::ios::binary};
            io::packed::read(vocab, num_words);
        }

        // two vectors for each word (target and context vectors)
        // each has vector_size number of features, plus one bias weight
        auto size = num_words * 2 * (vector_size_ + 1);
        weights_.resize(size);
        grad_squared_.resize(size, 1.0);

        // randomly initialize the word embeddings and biases
        {
            std::mt19937 engine{std::random_device{}()};
            std::generate(weights_.begin(), weights_.end(), [&]()
                          {
                              // use the word2vec style initialization
                              // I'm not entirely sure why, but this seems
                              // to do better than initializing the vectors
                              // to lie in the unit cube. Maybe scaling?
                              auto rnd = random::bounded_rand(engine, 65536);
                              return (rnd / 65536.0 - 0.5) / (vector_size_ + 1);
                          });
        }

        // shuffle the data and partition it into equal parts for each
        // thread
        auto total_records = shuffle_partition(prefix, max_ram, num_threads);

        // train using the specified number of threads
        train(prefix, num_threads, iters, total_records);

        // delete the temporary shuffled coocurrence files
        for (std::size_t i = 0; i < num_threads; ++i)
            filesystem::delete_file(prefix + "/coocur-shuf." + std::to_string(i)
                                    + ".bin");

        // save the target and context word embeddings
        save(prefix, num_words, num_rare);
    }

    util::array_view<double> target_vector(uint64_t term)
    {
        return {weights_.data() + (term * 2 * (vector_size_ + 1)),
                vector_size_};
    }

    util::array_view<const double> target_vector(uint64_t term) const
    {
        return {weights_.data() + (term * 2 * (vector_size_ + 1)),
                vector_size_};
    }

    double& target_bias(uint64_t term)
    {
        return weights_[term * 2 * (vector_size_ + 1) + vector_size_];
    }

    double target_bias(uint64_t term) const
    {
        return weights_[term * 2 * (vector_size_ + 1) + vector_size_];
    }

    util::array_view<double> context_vector(uint64_t term)
    {
        return {weights_.data() + (term * 2 + 1) * (vector_size_ + 1),
                vector_size_};
    }

    util::array_view<const double> context_vector(uint64_t term) const
    {
        return {weights_.data() + (term * 2 + 1) * (vector_size_ + 1),
                vector_size_};
    }

    double& context_bias(uint64_t term)
    {
        return weights_[(term * 2 + 1) * (vector_size_ + 1) + vector_size_];
    }

    double context_bias(uint64_t term) const
    {
        return weights_[(term * 2 + 1) * (vector_size_ + 1) + vector_size_];
    }

    double score(uint64_t target, uint64_t context) const
    {
        auto tv = target_vector(target);
        auto cv = context_vector(context);

        return std::inner_product(tv.begin(), tv.end(), cv.begin(), 0.0)
               + target_bias(target) + context_bias(context);
    }

  private:
    util::array_view<double> target_gradsq(uint64_t term)
    {
        return {grad_squared_.data() + (term * 2 * (vector_size_ + 1)),
                vector_size_};
    }

    double& target_bias_gradsq(uint64_t term)
    {
        return grad_squared_[term * 2 * (vector_size_ + 1) + vector_size_];
    }

    util::array_view<double> context_gradsq(uint64_t term)
    {
        return {grad_squared_.data() + (term * 2 + 1) * (vector_size_ + 1),
                vector_size_};
    }

    double& context_bias_gradsq(uint64_t term)
    {
        return grad_squared_[(term * 2 + 1) * (vector_size_ + 1)
                             + vector_size_];
    }

    void train(const std::string& prefix, std::size_t num_threads,
               std::size_t iters, std::size_t total_records)
    {
        parallel::thread_pool pool{num_threads};
        for (std::size_t i = 1; i <= iters; ++i)
        {
            printing::progress progress{" > Iteration: ", total_records};
            std::atomic_size_t records{0};
            std::vector<std::future<double>> futures;
            futures.reserve(num_threads);
            for (std::size_t t = 0; t < num_threads; ++t)
            {
                futures.emplace_back(pool.submit_task(
                    [&, t]()
                    {
                        return train_thread(prefix, t, progress, records);
                    }));
            }

            double total_cost = 0.0;
            auto elapsed = common::time([&]()
                                        {
                                            for (auto& fut : futures)
                                                total_cost += fut.get();
                                        });
            progress.end();

            LOG(progress) << "> Iteration " << i << "/" << iters
                          << ": avg cost = " << total_cost / total_records
                          << ", " << elapsed.count() / 1000.0 << " seconds\n"
                          << ENDLG;
        }
    }

    double cost_weight(double coocur) const
    {
        return (coocur < xmax_) ? std::pow(coocur / xmax_, scale_) : 1.0;
    }

    void update_weight(double* weight, double* gradsq, double grad)
    {
        // adaptive gradient update
        *weight -= grad / std::sqrt(*gradsq);
        *gradsq += grad * grad;
    }

    double train_thread(const std::string& prefix, std::size_t thread_id,
                        printing::progress& progress,
                        std::atomic_size_t& records)
    {
        using namespace embeddings;

        coocur_iterator iter{prefix + "/coocur-shuf."
                             + std::to_string(thread_id) + ".bin"};

        double cost = 0.0;

        for (; iter != coocur_iterator{}; ++iter)
        {
            progress(records++);
            auto record = *iter;

            auto diff = score(record.target, record.context)
                        - std::log(record.weight);
            auto weighted_diff = cost_weight(record.weight) * diff;

            // cost is weighted squared difference
            cost += 0.5 * weighted_diff * diff;

            auto delta = weighted_diff * learning_rate_;

            auto target = target_vector(record.target);
            auto targ_gradsq = target_gradsq(record.target);
            auto context = context_vector(record.context);
            auto ctx_gradsq = context_gradsq(record.context);

            auto target_it = target.begin();
            auto target_grad_it = targ_gradsq.begin();
            auto context_it = context.begin();
            auto context_grad_it = ctx_gradsq.begin();
            auto target_end = target.end();

            // update the embedding vectors
            while (target_it != target_end)
            {
                auto target_grad = delta * *context_it;
                auto context_grad = delta * *target_it;
                update_weight(target_it, target_grad_it, target_grad);
                update_weight(context_it, context_grad_it, context_grad);
                ++target_it;
                ++target_grad_it;
                ++context_it;
                ++context_grad_it;
            }

            // update the bias terms
            update_weight(&target_bias(record.target),
                          &target_bias_gradsq(record.target), delta);
            update_weight(&context_bias(record.context),
                          &context_bias_gradsq(record.context), delta);
        }

        return cost;
    }

    void save(const std::string& prefix, uint64_t num_words,
              uint64_t num_rare) const
    {
        // target embeddings
        {
            std::ofstream output{prefix + "/embeddings.target.bin",
                                 std::ios::binary};
            printing::progress progress{" > Saving target embeddings: ",
                                        num_words};
            io::packed::write(output, vector_size_);
            save_embeddings(output, num_words, num_rare, progress,
                            [&](uint64_t term)
                            {
                                return target_vector(term);
                            });
        }

        // context embeddings
        {

            std::ofstream output{prefix + "/embeddings.context.bin",
                                 std::ios::binary};
            printing::progress progress{" > Saving context embeddings: ",
                                        num_words};
            io::packed::write(output, vector_size_);
            save_embeddings(output, num_words, num_rare, progress,
                            [&](uint64_t term)
                            {
                                return context_vector(term);
                            });
        }
    }

    template <class VectorFetcher>
    void save_embeddings(std::ofstream& output, uint64_t num_words,
                         uint64_t num_rare, printing::progress& progress,
                         VectorFetcher&& vf) const
    {
        for (uint64_t tid = 0; tid < num_words; ++tid)
        {
            progress(tid);
            const auto& vec = vf(tid);
            write_normalized(vec.begin(), vec.end(), output);
        }

        util::aligned_vector<double> unk_vec(vector_size_, 0.0);
        auto num_to_average = std::min(num_rare, num_words);
        for (uint64_t tid = num_words - num_rare; tid < num_words; ++tid)
        {
            const auto& vec = vf(tid);
            std::transform(unk_vec.begin(), unk_vec.end(), vec.begin(),
                           unk_vec.begin(),
                           [=](double unkweight, double vecweight)
                           {
                               return unkweight + vecweight / num_to_average;
                           });
        }
        write_normalized(unk_vec.begin(), unk_vec.end(), output);
    }

    template <class ForwardIterator>
    void write_normalized(ForwardIterator begin, ForwardIterator end,
                          std::ofstream& output) const
    {
        auto len = std::sqrt(std::accumulate(begin, end, 0.0,
                                             [](double accum, double weight)
                                             {
                                                 return accum + weight * weight;
                                             }));
        std::for_each(begin, end, [&](double weight)
                      {
                          io::packed::write(output, weight / len);
                      });
    }

    util::aligned_vector<double> weights_;
    util::aligned_vector<double> grad_squared_;
    std::size_t vector_size_;
    double xmax_;
    double scale_;
    double learning_rate_;
};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);
    auto embed_cfg = config->get_table("embeddings");
    if (!embed_cfg)
    {
        std::cerr << "Missing [embeddings] configuration in " << argv[1]
                  << std::endl;
        return 1;
    }

    try
    {
        glove_trainer trainer{*embed_cfg};
    }
    catch (const glove_exception& ex)
    {
        LOG(fatal) << ex.what() << ENDLG;
        return 1;
    }
    return 0;
}
