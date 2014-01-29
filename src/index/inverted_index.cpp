/**
 * @file inverted_index.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include "corpus/corpus.h"
#include "index/chunk_handler.h"
#include "index/inverted_index.h"
#include "index/string_list.h"
#include "index/string_list_writer.h"
#include "index/vocabulary_map.h"
#include "index/vocabulary_map_writer.h"
#include "parallel/thread_pool.h"
#include "tokenizers/tokenizer.h"
#include "util/mapping.h"
#include "util/shim.h"

namespace meta {
namespace index {

inverted_index::inverted_index(const cpptoml::toml_group & config):
    disk_index{config},
    _index_name{*cpptoml::get_as<std::string>(config, "inverted-index")}
{ /* nothing */ }

inverted_index::inverted_index(inverted_index&&) = default;
inverted_index::~inverted_index() = default;
inverted_index& inverted_index::operator=(inverted_index&&) = default;

std::string inverted_index::index_name() const
{
    return _index_name;
}

void inverted_index::create_index(const std::string & config_file)
{
    // save the config file so we can recreate the tokenizer
    filesystem::copy_file(config_file, _index_name + "/config.toml");

    LOG(info) << "Creating index: " << _index_name << ENDLG;

    // load the documents from the corpus
    auto docs = corpus::corpus::load(config_file);

    uint64_t num_docs = docs->size();

    _doc_sizes = make_unique<util::disk_vector<double>>(
        _index_name + "/docsizes.counts", num_docs);
    _labels = make_unique<util::disk_vector<label_id>>(
        _index_name + "/docs.labels", num_docs);
    _unique_terms = make_unique<util::disk_vector<uint64_t>>(
        _index_name + "/docs.uniqueterms", num_docs);

    tokenize_docs(docs.get());

    _doc_id_mapping = make_unique<string_list>(_index_name
                                                       + "/docids.mapping");

    uint64_t num_unique_terms = merge_chunks(_index_name + "/postings.index");
    compress(_index_name + "/postings.index", num_unique_terms);

    _term_id_mapping =
        make_unique<vocabulary_map>(_index_name + "/termids.mapping");

    map::save_mapping(_label_ids, _index_name + "/labelids.mapping");
    _postings = make_unique<io::mmap_file>(_index_name + "/postings.index");

    LOG(info) << "Done creating index: " << _index_name << ENDLG;
}

void inverted_index::load_index()
{
    LOG(info) << "Loading index from disk: " << _index_name << ENDLG;

    auto config = cpptoml::parse_file(_index_name + "/config.toml");

    _doc_id_mapping = make_unique<string_list>(_index_name + "/docids.mapping");

    _term_id_mapping =
        make_unique<vocabulary_map>(_index_name + "/termids.mapping");

    _doc_sizes = make_unique<util::disk_vector<double>>(
        _index_name + "/docsizes.counts");
    _labels = make_unique<util::disk_vector<label_id>>(
        _index_name + "/docs.labels");
    _unique_terms = make_unique<util::disk_vector<uint64_t>>(
        _index_name + "/docs.uniqueterms");
    _term_bit_locations = make_unique<util::disk_vector<uint64_t>>(
        _index_name + "/lexicon.index");

    map::load_mapping(_label_ids, _index_name + "/labelids.mapping");
    _tokenizer = tokenizers::tokenizer::load(config);

    _postings = make_unique<io::mmap_file>(
        _index_name + "/postings.index"
    );
}

void inverted_index::tokenize_docs(corpus::corpus * docs)
{
    std::mutex mutex;
    std::atomic<uint32_t> chunk_num{0};
    string_list_writer doc_id_mapping{_index_name + "/docids.mapping",
                                            docs->size()};

    using namespace std::placeholders;
    auto writer = std::bind(&inverted_index::write_chunk, this, _1, _2);
    auto task = [&]() {
        index::chunk_handler<inverted_index> handler{this, chunk_num, writer};
        while (true) {
            util::optional<corpus::document> doc;
            {
                std::lock_guard<std::mutex> lock{mutex};

                if (!docs->has_next())
                    return; // destructor for handler will write
                            // any intermediate chunks
                doc = docs->next();

                std::string progress = "> Documents: "
                    + printing::add_commas(std::to_string(doc->id()))
                    + " Tokenizing: ";
                printing::show_progress(doc->id(), docs->size(), 1000, progress);
            }

            _tokenizer->tokenize(*doc);

            // save metadata
            doc_id_mapping.insert(doc->id(), doc->path());
            (*_doc_sizes)[doc->id()] = doc->length();
            (*_unique_terms)[doc->id()] = doc->counts().size();
            (*_labels)[doc->id()] = get_label_id(doc->label());
            // update chunk
            //handler(*doc);
            handler(doc->id(), doc->counts());
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

void inverted_index::write_chunk(uint32_t chunk_num,
                                 std::vector<index_pdata_type> & pdata)
{
    using chunk_t = chunk<std::string, secondary_key_type>;
    util::optional<chunk_t> top;
    {
        std::lock_guard<std::mutex> lock{*_queue_mutex};
        if(!_chunks.empty())
        {
            top = _chunks.top();
            _chunks.pop();
        }
    }

    if(!top) // pqueue was empty
    {
        std::string chunk_name =
            _index_name + "/chunk-" + std::to_string(chunk_num);
        io::compressed_file_writer outfile{chunk_name,
                                           io::default_compression_writer_func};
        for(auto & p: pdata)
            outfile << p;

        outfile.close(); // close so we can read the file size in chunk ctr
        std::ofstream termfile{chunk_name + ".numterms"};
        termfile << pdata.size();
        pdata.clear();

        std::lock_guard<std::mutex> lock{*_queue_mutex};
        _chunks.emplace(chunk_name);
    }
    else // we can merge with an existing chunk
    {
        top->memory_merge_with(pdata);

        std::lock_guard<std::mutex> lock{*_queue_mutex};
        _chunks.emplace(*top);
    }
}

uint64_t inverted_index::merge_chunks(const std::string & filename)
{
    using chunk_t = chunk<std::string, secondary_key_type>;

    // this represents the number of merge steps needed---it is equivalent
    // to the number of internal nodes in a binary tree with n leaf nodes
    size_t remaining = _chunks.size() - 1;
    std::mutex mutex;
    auto task = [&]() {
        while (true) {
            util::optional<chunk_t> first;
            util::optional<chunk_t> second;
            {
                std::lock_guard<std::mutex> lock{mutex};
                if (_chunks.size() < 2)
                    return;
                first = util::optional<chunk_t>{_chunks.top()};
                _chunks.pop();
                second = util::optional<chunk_t>{_chunks.top()};
                _chunks.pop();
                LOG(progress) << "> Merging " << first->path() << " ("
                    << printing::bytes_to_units(first->size())
                    << ") and " << second->path() << " ("
                    << printing::bytes_to_units(second->size())
                    << "), " << --remaining << " remaining        \r" << ENDLG;
            }
            first->merge_with(*second);
            {
                std::lock_guard<std::mutex> lock{mutex};
                _chunks.push(*first);
            }
        }
    };

    parallel::thread_pool pool;
    auto thread_ids = pool.thread_ids();
    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < thread_ids.size(); ++i)
        futures.emplace_back(pool.submit_task(task));

    for (auto & fut : futures)
        fut.get();

    LOG(progress) << '\n' << ENDLG;

    uint64_t num_unique_terms;
    std::ifstream termfile{_chunks.top().path() + ".numterms"};
    termfile >> num_unique_terms;
    termfile.close();
    filesystem::delete_file(_chunks.top().path() + ".numterms");
    filesystem::rename_file(_chunks.top().path(), filename);

    LOG(info) << "Created uncompressed postings file " << filename
              << " (" << printing::bytes_to_units(_chunks.top().size()) << ")"
              << ENDLG;

    return num_unique_terms;
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

        vocabulary_map_writer vocab{_index_name + "/termids.mapping"};

        postings_data<std::string, doc_id> pdata;
        auto length = filesystem::file_size(filename) * 8; // number of bits
        io::compressed_file_reader in{filename,
                                      io::default_compression_reader_func};
        auto idx = in.bit_location();

        // allocate memory for the term_id -> term location mapping now
        // that we know how many terms there are
        _term_bit_locations = make_unique<util::disk_vector<uint64_t>>(
                _index_name + "/lexicon.index", num_unique_terms);

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
        for(auto & sz: *_doc_sizes)
            _total_corpus_terms += sz;
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
    return static_cast<double>(total_corpus_terms()) / _doc_sizes->size();
}

void inverted_index::tokenize(corpus::document & doc)
{
    _tokenizer->tokenize(doc);
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

    io::compressed_file_reader reader{*_postings,
                                      io::default_compression_reader_func};
    reader.seek(_term_bit_locations->at(idx));

    auto pdata = std::make_shared<postings_data_type>(t_id);
    pdata->read_compressed(reader);

    return pdata;
}

}
}
