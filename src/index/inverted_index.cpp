/**
 * @file inverted_index.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include "meta/analyzers/analyzer.h"
#include "meta/corpus/corpus.h"
#include "meta/corpus/corpus_factory.h"
#include "meta/corpus/metadata_parser.h"
#include "meta/index/disk_index_impl.h"
#include "meta/index/inverted_index.h"
#include "meta/index/metadata_writer.h"
#include "meta/index/postings_file.h"
#include "meta/index/postings_file_writer.h"
#include "meta/index/postings_inverter.h"
#include "meta/index/vocabulary_map.h"
#include "meta/index/vocabulary_map_writer.h"
#include "meta/logging/logger.h"
#include "meta/parallel/thread_pool.h"
#include "meta/util/mapping.h"
#include "meta/util/pimpl.tcc"
#include "meta/util/printing.h"
#include "meta/util/progress.h"
#include "meta/util/shim.h"

namespace meta
{
namespace index
{

/**
 * Implementation of an inverted_index.
 */
class inverted_index::impl
{
  private:
    /// Pointer to the inverted_index this is an implementation of
    inverted_index* idx_;

  public:
    /**
     * Constructs an inverted_index impl.
     * @param parent The parent of this impl
     * @param config The config group
     */
    impl(inverted_index* parent, const cpptoml::table& config);

    /**
     * @param docs The documents to be tokenized
     * @param inverter The postings inverter for this index
     * @param mdata_parser The parser for reading metadata
     * @param mdata_writer The writer for metadata
     * @param ram_budget The total **estimated** RAM budget
     * @param num_threads The number of threads to tokenize and index docs with
     * @return the number of chunks created
     */
    void tokenize_docs(corpus::corpus& docs,
                       postings_inverter<inverted_index>& inverter,
                       metadata_writer& mdata_writer, uint64_t ram_budget,
                       uint64_t num_threads);

    /**
     * Compresses the large postings file.
     */
    void compress(const std::string& filename, uint64_t num_unique_terms);

    /**
     * Loads the postings file.
     */
    void load_postings();

    /// The analyzer used to tokenize documents.
    std::unique_ptr<analyzers::analyzer> analyzer_;

    util::optional<postings_file<inverted_index::primary_key_type,
                                 inverted_index::secondary_key_type>> postings_;

    /// the total number of term occurrences in the entire corpus
    uint64_t total_corpus_terms_;
};

inverted_index::impl::impl(inverted_index* idx, const cpptoml::table& config)
    : idx_{idx}, analyzer_{analyzers::load(config)}, total_corpus_terms_{0}
{
    // nothing
}

inverted_index::inverted_index(const cpptoml::table& config)
    : disk_index{config, *config.get_as<std::string>("index") + "/inv"},
      inv_impl_{this, config}
{
    // nothing
}

inverted_index::inverted_index(inverted_index&&) = default;
inverted_index& inverted_index::operator=(inverted_index&&) = default;
inverted_index::~inverted_index() = default;

bool inverted_index::valid() const
{
    for (auto& f : impl_->files)
    {
        if (!filesystem::file_exists(index_name() + "/" + std::string{f}))
        {
            LOG(info)
                << "Existing inverted index detected as invalid; recreating"
                << ENDLG;
            return false;
        }
    }
    return true;
}

void inverted_index::create_index(const cpptoml::table& config,
                                  corpus::corpus& docs)
{
    if (!filesystem::make_directories(index_name()))
        throw exception{"Unable to create index directory: " + index_name()};

    // save the config file so we can recreate the analyzer
    {
        std::ofstream config_file{index_name() + "/config.toml"};
        config_file << config;
    }

    LOG(info) << "Creating index: " << index_name() << ENDLG;

    auto ram_budget = static_cast<uint64_t>(
        config.get_as<int64_t>("indexer-ram-budget").value_or(1024));
    auto max_writers = static_cast<unsigned>(
        config.get_as<int64_t>("indexer-max-writers").value_or(8));

    auto max_threads = std::thread::hardware_concurrency();
    auto num_threads = static_cast<unsigned>(
        config.get_as<int64_t>("indexer-num-threads").value_or(max_threads));
    if (num_threads > max_threads)
    {
        num_threads = max_threads;
        LOG(warning) << "Reducing indexer-num-threads to the hardware "
                        "concurrency level of "
                     << max_threads << ENDLG;
    }

    postings_inverter<inverted_index> inverter{index_name(), max_writers};
    {
        metadata_writer mdata_writer{index_name(), docs.size(), docs.schema()};
        uint64_t num_docs = docs.size();
        impl_->load_labels(num_docs);

        // RAM budget is given in megabytes
        inv_impl_->tokenize_docs(docs, inverter, mdata_writer,
                                 ram_budget * 1024 * 1024, num_threads);
    }

    inverter.merge_chunks();

    LOG(info) << "Created uncompressed postings file " << index_name()
              << impl_->files[POSTINGS] << " ("
              << printing::bytes_to_units(inverter.final_size()) << ")"
              << ENDLG;

    uint64_t num_unique_terms = inverter.unique_primary_keys();
    inv_impl_->compress(index_name() + impl_->files[POSTINGS],
                        num_unique_terms);

    impl_->load_term_id_mapping();
    impl_->initialize_metadata();

    // reload the label file to ensure it flushed
    impl_->load_labels();

    impl_->save_label_id_mapping();
    inv_impl_->load_postings();

    LOG(info) << "Done creating index: " << index_name() << ENDLG;
}

void inverted_index::load_index()
{
    LOG(info) << "Loading index from disk: " << index_name() << ENDLG;

    impl_->initialize_metadata();
    impl_->load_term_id_mapping();
    impl_->load_label_id_mapping();
    impl_->load_labels();
    inv_impl_->load_postings();
}

void inverted_index::impl::tokenize_docs(
    corpus::corpus& docs, postings_inverter<inverted_index>& inverter,
    metadata_writer& mdata_writer, uint64_t ram_budget, uint64_t num_threads)
{
    std::mutex mutex;
    printing::progress progress{" > Tokenizing Docs: ", docs.size()};

    auto task = [&](uint64_t ram_budget)
    {
        auto producer = inverter.make_producer(ram_budget);
        auto analyzer = analyzer_->clone();
        while (true)
        {
            util::optional<corpus::document> doc;
            {
                std::lock_guard<std::mutex> lock{mutex};

                if (!docs.has_next())
                    return; // destructor for producer will write
                            // any intermediate chunks
                doc = docs.next();
                progress(doc->id());
            }

            auto counts = analyzer->analyze<uint64_t>(*doc);

            // warn if there is an empty document
            if (counts.empty())
            {
                std::lock_guard<std::mutex> lock{mutex};
                LOG(progress) << '\n' << ENDLG;
                LOG(warning) << "Empty document (id = " << doc->id()
                             << ") generated!" << ENDLG;
            }

            auto length = std::accumulate(
                counts.begin(), counts.end(), 0ul,
                [](uint64_t acc, const std::pair<std::string, uint64_t>& count)
                {
                    return acc + count.second;
                });

            mdata_writer.write(doc->id(), length, counts.size(), doc->mdata());
            idx_->impl_->set_label(doc->id(), doc->label());

            // update chunk
            producer(doc->id(), counts);
        }
    };

    parallel::thread_pool pool{num_threads};
    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < num_threads; ++i)
    {
        futures.emplace_back(
            pool.submit_task(std::bind(task, ram_budget / num_threads)));
    }

    for (auto& fut : futures)
        fut.get();
}

void inverted_index::impl::compress(const std::string& filename,
                                    uint64_t num_unique_terms)
{
    std::string ucfilename{filename + ".uncompressed"};
    filesystem::rename_file(filename, ucfilename);

    // create a scope to ensure the reader and writer close properly so we
    // can calculate the size of the compressed file and delete the
    // uncompressed version at the end
    {
        postings_file_writer<inverted_index::index_pdata_type> out{
            filename, num_unique_terms};

        vocabulary_map_writer vocab{idx_->index_name()
                                    + idx_->impl_->files[TERM_IDS_MAPPING]};

        inverted_index::index_pdata_type pdata;
        auto length = filesystem::file_size(ucfilename);
        std::ifstream in{ucfilename, std::ios::binary};
        uint64_t byte_pos = 0;

        printing::progress progress{" > Compressing postings: ", length};
        // note: we will be accessing pdata in sorted order
        while (auto bytes = pdata.read_packed(in))
        {
            byte_pos += bytes;
            progress(byte_pos);
            vocab.insert(pdata.primary_key());
            out.write(pdata);
        }
    }

    LOG(info) << "Created compressed postings file ("
              << printing::bytes_to_units(filesystem::file_size(filename))
              << ")" << ENDLG;

    filesystem::delete_file(ucfilename);
}

void inverted_index::impl::load_postings()
{
    postings_ = {idx_->index_name() + idx_->impl_->files[POSTINGS]};
}

uint64_t inverted_index::term_freq(term_id t_id, doc_id d_id) const
{
    auto pdata = search_primary(t_id);
    return pdata->count(d_id);
}

uint64_t inverted_index::total_corpus_terms()
{
    if (inv_impl_->total_corpus_terms_ == 0)
    {
        for (auto& id : docs())
            inv_impl_->total_corpus_terms_ += doc_size(id);
    }

    return inv_impl_->total_corpus_terms_;
}

uint64_t inverted_index::total_num_occurences(term_id t_id) const
{
    auto pdata = search_primary(t_id);

    double sum = 0;
    for (auto& c : pdata->counts())
        sum += c.second;

    return static_cast<uint64_t>(sum);
}

float inverted_index::avg_doc_length()
{
    return static_cast<float>(total_corpus_terms()) / num_docs();
}

analyzers::feature_map<uint64_t>
inverted_index::tokenize(const corpus::document& doc)
{
    return inv_impl_->analyzer_->analyze<uint64_t>(doc);
}

uint64_t inverted_index::doc_freq(term_id t_id) const
{
    return search_primary(t_id)->counts().size();
}

auto inverted_index::search_primary(term_id t_id) const
    -> std::shared_ptr<postings_data_type>
{
    return inv_impl_->postings_->find(t_id);
}

util::optional<postings_stream<doc_id>>
inverted_index::stream_for(term_id t_id) const
{
    return inv_impl_->postings_->find_stream(t_id);
}
}
}
