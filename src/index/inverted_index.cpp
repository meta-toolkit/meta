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
#include "tokenizers/tokenizer.h"
#include "util/mapping.h"
#include "util/pimpl.tcc"
#include "util/shim.h"

namespace meta {
namespace index {

inverted_index::inverted_index(const cpptoml::toml_group & config):
    disk_index{config, *cpptoml::get_as<std::string>(config, "inverted-index")},
    tokenizer_{tokenizers::tokenizer::load(config)}
{ /* nothing */ }

inverted_index::inverted_index(inverted_index&&) = default;
inverted_index& inverted_index::operator=(inverted_index&&) = default;
inverted_index::~inverted_index() = default;

void inverted_index::create_index(const std::string & config_file)
{
    // save the config file so we can recreate the tokenizer
    filesystem::copy_file(config_file, index_name() + "/config.toml");

    LOG(info) << "Creating index: " << index_name() << ENDLG;

    // load the documents from the corpus
    auto docs = corpus::corpus::load(config_file);

    uint64_t num_docs = docs->size();
    impl_->initialize_metadata(num_docs);

    chunk_handler<inverted_index> handler{index_name()};
    tokenize_docs(docs.get(), handler);

    impl_->load_doc_id_mapping();

    handler.merge_chunks();

    LOG(info) << "Created uncompressed postings file " << index_name()
              << impl_->files[POSTINGS]
              << " (" << printing::bytes_to_units(handler.final_size()) << ")"
              << ENDLG;

    uint64_t num_unique_terms = handler.unique_primary_keys();
    compress(index_name() + impl_->files[POSTINGS], num_unique_terms);

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

    _term_bit_locations = make_unique<util::disk_vector<uint64_t>>(
        index_name() + "/lexicon.index");

    impl_->load_label_id_mapping();
    impl_->load_postings();
}

void inverted_index::tokenize_docs(corpus::corpus * docs,
                                   chunk_handler<inverted_index> & handler)
{
    std::mutex mutex;
    auto docid_writer = impl_->make_doc_id_writer(docs->size());

    auto task = [&]() {
        auto producer = handler.make_producer();
        while (true) {
            util::optional<corpus::document> doc;
            {
                std::lock_guard<std::mutex> lock{mutex};

                if (!docs->has_next())
                    return; // destructor for producer will write
                            // any intermediate chunks
                doc = docs->next();

                std::string progress = "> Documents: "
                    + printing::add_commas(std::to_string(doc->id()))
                    + " Tokenizing: ";
                printing::show_progress(doc->id(), docs->size(), 1000, progress);
            }

            tokenize(*doc);

            // save metadata
            docid_writer.insert(doc->id(), doc->path());
            impl_->set_length(doc->id(), doc->length());
            impl_->set_unique_terms(doc->id(), doc->counts().size());
            impl_->set_label(doc->id(), doc->label());
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

    std::string progress = "> Documents: "
        + printing::add_commas(std::to_string(docs->size()))
        + " Tokenizing: ";
    printing::end_progress(progress);
}

void inverted_index::compress(const std::string & filename,
        uint64_t num_unique_terms)
{
    std::string cfilename{filename + ".compressed"};

    // create scope so the writer closes and we can calculate the size of the
    // file as well as rename it
    {
        io::compressed_file_writer out{cfilename,
                                       io::default_compression_writer_func};

        vocabulary_map_writer vocab{index_name() +
                                    impl_->files[TERM_IDS_MAPPING]};

        postings_data<std::string, doc_id> pdata;
        auto length = filesystem::file_size(filename) * 8; // number of bits
        io::compressed_file_reader in{filename,
                                      io::default_compression_reader_func};
        auto idx = in.bit_location();

        // allocate memory for the term_id -> term location mapping now
        // that we know how many terms there are
        _term_bit_locations = make_unique<util::disk_vector<uint64_t>>(
                index_name() + "/lexicon.index", num_unique_terms);

        // note: we will be accessing pdata in sorted order
        term_id t_id{0};
        while(in.has_next())
        {
            in >> pdata;
            if (idx != in.bit_location() / (length / 500))
            {
                idx = in.bit_location() / (length / 500);
                printing::show_progress(idx, 500, 1,
                        " > Creating compressed postings file: ");
            }
            vocab.insert(pdata.primary_key());
            (*_term_bit_locations)[t_id] = out.bit_location();
            pdata.write_compressed(out);
            ++t_id;
        }
        printing::end_progress(" > Creating compressed postings file: ");
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
    if(_total_corpus_terms == 0)
    {
        for(auto & id: docs())
            _total_corpus_terms += doc_size(id);
    }

    return _total_corpus_terms;
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
    tokenizer_->tokenize(doc);
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
    if(idx >= _term_bit_locations->size())
        return std::make_shared<postings_data_type>(t_id);

    io::compressed_file_reader reader{impl_->postings(),
                                      io::default_compression_reader_func};
    reader.seek(_term_bit_locations->at(idx));

    auto pdata = std::make_shared<postings_data_type>(t_id);
    pdata->read_compressed(reader);

    return pdata;
}

}
}
