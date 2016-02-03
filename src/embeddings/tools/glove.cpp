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
#include "meta/util/aligned_allocator.h"
#include "meta/util/progress.h"
#include "meta/util/printing.h"
#include "meta/util/random.h"

using namespace meta;

template <class T>
class array_view
{
  public:
    array_view(T* start, std::size_t len) : start_{start}, end_{start + len}
    {
        // nothing
    }

    array_view(T* start, T* end) : start_{start}, end_{end}
    {
        // nothing
    }

    T* begin() const
    {
        return start_;
    }

    T* end() const
    {
        return end_;
    }

    std::size_t size() const
    {
        return end_ - start_;
    }

  private:
    T* start_;
    T* end_;
};

class glove
{
  public:
    glove(std::size_t num_words, std::size_t vector_size)
        : weights_(num_words * 2 * (vector_size + 1)), vector_size_{vector_size}
    {
        // two vectors for each word (target and context vectors)
        // each has vector_size number of features, plus one bias weight
    }

    array_view<double> target_vector(uint64_t term)
    {
        return {weights_.data() + (term * 2 * (vector_size_ + 1)),
                vector_size_};
    }

    array_view<const double> target_vector(uint64_t term) const
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

    array_view<double> context_vector(uint64_t term)
    {
        return {weights_.data() + (term * 2 * (vector_size_ + 1)) + vector_size_
                    + 1,
                vector_size_};
    }

    array_view<const double> context_vector(uint64_t term) const
    {
        return {weights_.data() + (term * 2 * (vector_size_ + 1)) + vector_size_
                    + 1,
                vector_size_};
    }

    double& context_bias(uint64_t term)
    {
        return weights_[term * 2 * (vector_size_ + 1) + 2 * vector_size_ + 1];
    }

    double context_bias(uint64_t term) const
    {
        return weights_[term * 2 * (vector_size_ + 1) + 2 * vector_size_ + 1];
    }

    double score(uint64_t target, uint64_t context) const
    {
        auto tv = target_vector(target);
        auto cv = context_vector(context);

        return std::inner_product(tv.begin(), tv.end(), cv.begin(), 0.0)
               + target_bias(target) + context_bias(context);
    }

  private:
    util::aligned_vector<double> weights_;
    std::size_t vector_size_;
};

void shuffle_partition(const std::string& prefix, std::size_t max_ram,
                       std::size_t num_partitions)
{
    using namespace embeddings;

    using vec_type = std::vector<coocur_record>;
    using diff_type = vec_type::iterator::difference_type;

    std::mt19937 engine{std::random_device{}()};
    vec_type records(max_ram / sizeof(coocur_record));

    // read in RAM sized chunks and shuffle in memory and write out to disk
    std::vector<uint64_t> chunk_sizes;

    uint64_t total_records = 0;
    coocur_iterator input{prefix + "/coocur.bin"};

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
                         records.begin() + static_cast<diff_type>(i), engine);

            std::ofstream output{prefix + "/coocur-shuf."
                                     + std::to_string(chunk_sizes.size())
                                     + ".tmp",
                                 std::ios::binary};

            total_records += i;
            chunk_sizes.push_back(i);
            for (std::size_t j = 0; j < i; ++j)
                records[j].write(output);
        }
    }

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
                records[j].write(outputs[idx]);
            }
        }
    }

    // delete temporary files
    for (std::size_t i = 0; i < chunk_sizes.size(); ++i)
    {
        filesystem::delete_file(prefix + "/coocur-shuf." + std::to_string(i)
                                + ".tmp");
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    auto config = cpptoml::parse_file(argv[1]);

    // extract building parameters
    auto embed_cfg = config->get_table("embeddings");
    auto prefix = *embed_cfg->get_as<std::string>("prefix");
    auto max_ram = static_cast<std::size_t>(
                       embed_cfg->get_as<int64_t>("max-ram").value_or(4096))
                   * 1024 * 1024;

    if (!filesystem::file_exists(prefix + "/vocab.bin"))
    {
        LOG(fatal) << "Vocabulary has not yet been generated, please "
                      "do this before learning word embeddings"
                   << ENDLG;
        return 1;
    }

    if (!filesystem::file_exists(prefix + "/coocur.bin"))
    {
        LOG(fatal) << "Coocurrence matrix has not yet been generated, please "
                      "do this before learning word embeddings"
                   << ENDLG;
        return 1;
    }

    uint64_t num_words = 0;
    {
        std::ifstream vocab{prefix + "/vocab.bin", std::ios::binary};
        io::packed::read(vocab, num_words);
    }

    std::size_t partitions = std::max(std::thread::hardware_concurrency(), 1u);

    shuffle_partition(prefix, max_ram, partitions);
}
