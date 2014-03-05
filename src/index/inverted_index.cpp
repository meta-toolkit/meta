/**
 * @file inverted_index.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include "corpus/corpus.h"
#include "index/chunk_handler.h"
#include "index/disk_index_impl.h"
#include "index/inverted_index.h"
#include "index/string_list.h"
#include "index/string_list_writer.h"
#include "index/vocabulary_map.h"
#include "index/vocabulary_map_writer.h"
#include "parallel/thread_pool.h"
#include "analyzers/analyzer.h"
#include "util/mapping.h"
#include "util/pimpl.tcc"
#include "util/progress.h"
#include "util/shim.h"

namespace meta {
namespace index {

class inverted_index::impl
{
  private:
    inverted_index* idx_;

  public:
    impl(inverted_index* parent, const cpptoml::toml_group& config);

    /**
     * @param docs The documents to be tokenized
     * @return the number of chunks created
     */
    void tokenize_docs(corpus::corpus* docs,
                       chunk_handler<inverted_index>& handler);

    /**
     * Creates the lexicon file (or "dictionary") which has pointers into
     * the large postings file
     * @param postings_file
     * @param lexicon_file
     */
    void create_lexicon(const std::string& postings_file,
                        const std::string& lexicon_file);

    /**
     * Compresses the large postings file.
     */
    void compress(const std::string& filename, uint64_t num_unique_terms);

    /**
     * The analyzer used to tokenize documents.
     */
    std::unique_ptr<analyzers::analyzer> analyzer_;

    /**
     * PrimaryKey -> postings location.
     * Each index corresponds to a PrimaryKey (uint64_t).
     */
    util::optional<util::disk_vector<uint64_t>> _term_bit_locations;

    /** the total number of term occurrences in the entire corpus */
    uint64_t _total_corpus_terms;
};

inverted_index::impl::impl(inverted_index* idx,
                           const cpptoml::toml_group& config)
    : idx_{idx},
      analyzer_{analyzers::analyzer::load(config)},
      _total_corpus_terms{0}
{
    // nothing
}

inverted_index::inverted_index(const cpptoml::toml_group & config):
    disk_index{config, *cpptoml::get_as<std::string>(config, "inverted-index")},
    inv_impl_{this, config}
{
    // nothing
}

inverted_index::inverted_index(inverted_index&&) = default;
inverted_index& inverted_index::operator=(inverted_index&&) = default;
inverted_index::~inverted_index() = default;

void inverted_index::create_index(const std::string & config_file)
{
    // save the config file so we can recreate the analyzer
    filesystem::copy_file(config_file, index_name() + "/config.toml");

    LOG(info) << "Creating index: " << index_name() << ENDLG;

    // load the documents from the corpus
    auto docs = corpus::corpus::load(config_file);

    uint64_t num_docs = docs->size();
    impl_->initialize_metadata(num_docs);

    chunk_handler<inverted_index> handler{index_name()};
    inv_impl_->tokenize_docs(docs.get(), handler);

    impl_->load_doc_id_mapping();

    handler.merge_chunks();

    LOG(info) << "Created uncompressed postings file " << index_name()
              << impl_->files[POSTINGS]
              << " (" << printing::bytes_to_units(handler.final_size()) << ")"
              << ENDLG;

    uint64_t num_unique_terms = handler.unique_primary_keys();
    inv_impl_->compress(index_name() + impl_->files[POSTINGS],
                        num_unique_terms);

    impl_->load_term_id_mapping();

    impl_->save_label_id_mapping();
    impl_->load_postings();

    LOG(info) << "Done creating index: " << index_name() << ENDLG;
}

void inverted_index::load_index()
{
    LOG(info) << "Loading index from disk: " << index_name() << ENDLG;

    auto config = cpptoml::parse_file(index_name() + "/config.toml");

    impl_->initialize_metadata();
    impl_->load_doc_id_mapping();
    impl_->load_term_id_mapping();

    inv_impl_->_term_bit_locations =
        util::disk_vector<uint64_t>(index_name() + "/lexicon.index");

    impl_->load_label_id_mapping();
    impl_->load_postings();
}

void inverted_index::impl::tokenize_docs(corpus::corpus* docs,
                                         chunk_handler<inverted_index>& handler)
{
    std::mutex mutex;
    auto docid_writer = idx_->impl_->make_doc_id_writer(docs->size());

    printing::progress progress{" > Tokenizing Docs: ", docs->size()};

    auto task = [&]() {
        auto producer = handler.make_producer();
        auto analyzer = analyzer_->clone();
        while (true) {
            util::optional<corpus::document> doc;
            {
                std::lock_guard<std::mutex> lock{mutex};

                if (!docs->has_next())
                    return; // destructor for producer will write
                            // any intermediate chunks
                doc = docs->next();
                progress(doc->id());
            }

            analyzer->tokenize(*doc);

            // save metadata
            docid_writer.insert(doc->id(), doc->path());
            idx_->impl_->set_length(doc->id(), doc->length());
            idx_->impl_->set_unique_terms(doc->id(), doc->counts().size());
            idx_->impl_->set_label(doc->id(), doc->label());
            // update chunk
            producer(doc->id(), doc->counts());
        }
    };

    parallel::thread_pool pool;
    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < pool.thread_ids().size(); ++i)
        futures.emplace_back(pool.submit_task(task));

    for (auto & fut : futures)
        fut.get();
}

void inverted_index::impl::compress(const std::string & filename,
        uint64_t num_unique_terms)
{
    std::string cfilename{filename + ".compressed"};

    // create scope so the writer closes and we can calculate the size of the
    // file as well as rename it
    {
        io::compressed_file_writer out{cfilename,
                                       io::default_compression_writer_func};

        vocabulary_map_writer vocab{idx_->index_name() +
                                    idx_->impl_->files[TERM_IDS_MAPPING]};

        postings_data<std::string, doc_id> pdata;
        auto length = filesystem::file_size(filename) * 8; // number of bits
        io::compressed_file_reader in{filename,
                                      io::default_compression_reader_func};

        // allocate memory for the term_id -> term location mapping now
        // that we know how many terms there are
        _term_bit_locations = util::disk_vector<uint64_t>(
            idx_->index_name() + "/lexicon.index", num_unique_terms);

        printing::progress progress{" > Compressing postings: ", length, 500,
                                    8 * 1024 /* 1KB */};
        // note: we will be accessing pdata in sorted order
        term_id t_id{0};
        while(in.has_next())
        {
            in >> pdata;
            progress(in.bit_location());
            vocab.insert(pdata.primary_key());
            (*_term_bit_locations)[t_id] = out.bit_location();
            pdata.write_compressed(out);
            ++t_id;
        }
    }

    LOG(info) << "Created compressed postings file ("
          << printing::bytes_to_units(filesystem::file_size(cfilename))
          << ")" << ENDLG;

    filesystem::delete_file(filename);
    filesystem::rename_file(cfilename, filename);
}

uint64_t inverted_index::term_freq(term_id t_id, doc_id d_id) const
{
    auto pdata = search_primary(t_id);
    return pdata->count(d_id);
}

uint64_t inverted_index::total_corpus_terms()
{
    if(inv_impl_->_total_corpus_terms == 0)
    {
        for(auto & id: docs())
            inv_impl_->_total_corpus_terms += doc_size(id);
    }

    return inv_impl_->_total_corpus_terms;
}

uint64_t inverted_index::total_num_occurences(term_id t_id) const
{
    auto pdata = search_primary(t_id);

    double sum = 0;
    for(auto & c: pdata->counts())
        sum += c.second;

    return sum;
}

double inverted_index::avg_doc_length()
{
    return static_cast<double>(total_corpus_terms())
           / num_docs();
}

void inverted_index::tokenize(corpus::document & doc)
{
    inv_impl_->analyzer_->tokenize(doc);
}

uint64_t inverted_index::doc_freq(term_id t_id) const
{
    return search_primary(t_id)->counts().size();
}

auto inverted_index::search_primary(term_id t_id) const
    -> std::shared_ptr<postings_data_type>
{
    uint64_t idx{t_id};

    // if the term doesn't exist in the index, return an empty postings_data
    if(idx >= inv_impl_->_term_bit_locations->size())
        return std::make_shared<postings_data_type>(t_id);

    io::compressed_file_reader reader{impl_->postings(),
                                      io::default_compression_reader_func};
    reader.seek(inv_impl_->_term_bit_locations->at(idx));

    auto pdata = std::make_shared<postings_data_type>(t_id);
    pdata->read_compressed(reader);

    return pdata;
}

}
}
