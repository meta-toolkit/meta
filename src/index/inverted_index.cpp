/**
 * @file inverted_index.cpp
 * @author Sean Massung
 */
#include <iostream>
#include <numeric>
#include "index/inverted_index.h"
#include "index/chunk.h"
#include "logging/logger.h"
#include "parallel/thread_pool.h"

namespace meta {
namespace index {

inverted_index::inverted_index(const cpptoml::toml_group & config):
    _tokenizer{ tokenizers::tokenizer::load(config) },
    _index_name{ *cpptoml::get_as<std::string>(config, "inverted-index") }
{ /* nothing */ }

void inverted_index::create_index(const std::string & config_file)
{
    // save the config file so we can recreate the tokenizer
    std::ifstream source_config{config_file, std::ios::binary};
    std::ofstream dest_config{_index_name + "/config.toml", std::ios::binary};
    dest_config << source_config.rdbuf();

    LOG(info) << "Creating index: " << _index_name << ENDLG;

    // load the documents from the corpus
    auto docs = corpus::corpus::load(config_file);

    uint64_t num_docs = docs->size();

    _doc_id_mapping =
        common::make_unique<util::sqlite_map<doc_id, std::string,
                                             caching::default_dblru_cache>>(
            _index_name + "/docids.mapping"
        );

    _term_id_mapping =
        common::make_unique<util::sqlite_map<std::string, uint64_t,
                                             caching::default_dblru_cache>>(
            _index_name + "/termids.mapping"
        );

    _doc_sizes = common::make_unique<util::disk_vector<double>>(
        _index_name + "/docsizes.counts", num_docs);
    _labels = common::make_unique<util::disk_vector<label_id>>(
        _index_name + "/docs.labels", num_docs);
    _unique_terms = common::make_unique<util::disk_vector<uint64_t>>(
        _index_name + "/docs.uniqueterms", num_docs);

    uint32_t num_chunks;
    auto time = common::time([&](){
        num_chunks = tokenize_docs(docs.get());
    });

    LOG(debug) << " ! Time spent tokenizing: " << (time.count() / 1000.0) << ENDLG;

    uint64_t num_unique_terms;
    time = common::time([&](){
        num_unique_terms = merge_chunks(_index_name + "/postings.index");
    });

    LOG(debug) << " ! Time spent merging: " << (time.count() / 1000.0) << ENDLG;

    time = common::time([&](){
        compress(_index_name + "/postings.index", num_unique_terms);
    });

    LOG(debug) << " ! Time spent compressing: " << (time.count() / 1000.0) << ENDLG;

    common::save_mapping(_label_ids, _index_name + "/labelids.mapping");

    _postings = common::make_unique<io::mmap_file>(_index_name + "/postings.index");

    LOG(info) << "Done creating index: " << _index_name << ENDLG;
}

void inverted_index::load_index()
{
    LOG(info) << "Loading index from disk: " << _index_name << ENDLG;

    auto config = cpptoml::parse_file(_index_name + "/config.toml");

    _doc_id_mapping =
        common::make_unique<util::sqlite_map<doc_id, std::string,
                                             caching::default_dblru_cache>>(
            _index_name + "/docids.mapping"
        );

    _term_id_mapping =
        common::make_unique<util::sqlite_map<std::string, uint64_t,
                                             caching::default_dblru_cache>>(
            _index_name + "/termids.mapping"
        );

    _doc_sizes = common::make_unique<util::disk_vector<double>>(
        _index_name + "/docsizes.counts");
    _labels = common::make_unique<util::disk_vector<label_id>>(
        _index_name + "/docs.labels");
    _unique_terms = common::make_unique<util::disk_vector<uint64_t>>(
        _index_name + "/docs.uniqueterms");
    _term_bit_locations = common::make_unique<util::disk_vector<uint64_t>>(
        _index_name + "/lexicon.index");

    common::load_mapping(_label_ids, _index_name + "/labelids.mapping");
    _tokenizer = tokenizers::tokenizer::load(config);

    _postings = common::make_unique<io::mmap_file>(
        _index_name + "/postings.index"
    );
}

uint32_t inverted_index::tokenize_docs(corpus::corpus * docs)
{
    std::mutex mutex;
    std::atomic<uint32_t> chunk_num{0};
    std::atomic<uint64_t> token_time{0};
    std::atomic<uint64_t> metadata_time{0};
    std::atomic<uint64_t> handler_time{0};
    uint64_t loading_docs_time{0};

    auto task = [&]() {
        chunk_handler handler{this, chunk_num};
        while (true) {
            util::optional<corpus::document> doc;
            {
                std::lock_guard<std::mutex> lock{mutex};

                if (!docs->has_next())
                    return; // destructor for handler will write
                            // any intermediate chunks
                auto time = common::time([&]() {
                    doc = docs->next();
                });
                loading_docs_time += time.count();

                std::string progress = "> Documents: "
                    + printing::add_commas(common::to_string(doc->id()))
                    + " Unique primary keys: "
                    + printing::add_commas(
                        common::to_string(_term_id_mapping->size()))
                    + " Tokenizing: ";
                printing::show_progress(doc->id(), docs->size(), 1000, progress);
            }

            auto time = common::time([&]() {
                _tokenizer->tokenize(*doc);
            });
            token_time.fetch_add(time.count());

            // save metadata
            time = common::time([&]() {
                _doc_id_mapping->insert(doc->id(), doc->path());
                (*_doc_sizes)[doc->id()] = doc->length();
                (*_unique_terms)[doc->id()] = doc->counts().size();
                (*_labels)[doc->id()] = get_label_id(doc->label());
            });
            metadata_time.fetch_add(time.count());
            // update chunk
            time = common::time([&]() {
                handler(*doc);
            });
            handler_time.fetch_add(time.count());
        }
    };

    parallel::thread_pool pool;
    std::vector<std::future<void>> futures;
    for (size_t i = 0; i < pool.thread_ids().size(); ++i)
        futures.emplace_back(pool.submit_task(task));

    for (auto & fut : futures)
        fut.get();

    std::string progress = "> Documents: "
        + printing::add_commas(common::to_string(docs->size()))
        + " Unique primary keys: "
        + printing::add_commas(common::to_string(_term_id_mapping->size()))
        + " Tokenizing: ";
    printing::end_progress(progress);

    LOG(debug) << "   ! CPU Time tokenizing: " << token_time / 1000.0
        << "s" << ENDLG;
    LOG(debug) << "   ! CPU Time metadata building: " << metadata_time / 1000.0
        << "s" << ENDLG;
    LOG(debug) << "   ! CPU Time in handler: " << handler_time / 1000.0
        << "s" << ENDLG;
    LOG(debug) << "   ! Wall time getting documents: " << loading_docs_time / 1000.0
        << "s" << ENDLG;

    return chunk_num;
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
            _index_name + "/chunk-" + common::to_string(chunk_num);
        io::compressed_file_writer outfile{chunk_name,
            common::default_compression_writer_func};
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
            common::default_compression_writer_func};

        postings_data<std::string, doc_id> pdata; // TODO: breaks forward_index
        auto length = filesystem::file_size(filename) * 8; // number of bits
        io::compressed_file_reader in{filename,
            common::default_compression_reader_func};
        auto idx = in.bit_location();

        // allocate memory for the term_id -> term location mapping now
        // that we know how many terms there are
        _term_bit_locations = common::make_unique<util::disk_vector<uint64_t>>(
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
            _term_id_mapping->insert(pdata.primary_key(), t_id);
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

term_id inverted_index::get_term_id(const std::string & term)
{
    std::lock_guard<std::mutex> lock{*_mutex};

    auto termID = _term_id_mapping->find(term);
    if(termID)
        return term_id{*termID};

    uint64_t size = _term_id_mapping->size();
    _term_id_mapping->insert(term, term_id{size});
    return term_id{size};
}

class_label inverted_index::label(doc_id d_id) const
{
    return class_label_from_id(_labels->at(d_id));
}

class_label inverted_index::class_label_from_id(label_id l_id) const
{
    return _label_ids.get_key(l_id);
}

label_id inverted_index::get_label_id(const class_label & lbl)
{
    std::lock_guard<std::mutex> lock{*_mutex};
    if(!_label_ids.contains_key(lbl))
    {
        label_id next_id{static_cast<label_id>(_label_ids.size())};
        _label_ids.insert(lbl, next_id);
        return next_id;
    }
    else
        return _label_ids.get_value(lbl);
}

label_id inverted_index::label_id_from_doc(doc_id d_id) const
{
    return _labels->at(d_id);
}

std::string inverted_index::index_name() const
{
    return _index_name;
}

uint64_t inverted_index::unique_terms(doc_id d_id) const
{
    return _unique_terms->at(d_id);
}

uint64_t inverted_index::unique_terms() const
{
    return _term_id_mapping->size();
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

uint64_t inverted_index::doc_size(doc_id d_id) const
{
    return _doc_sizes->at(d_id);
}

uint64_t inverted_index::num_docs() const
{
    return _doc_sizes->size();
}

std::string inverted_index::doc_name(doc_id d_id) const
{
    auto path = doc_path(d_id);
    return path.substr(path.find_last_of("/") + 1);
}

std::string inverted_index::doc_path(doc_id d_id) const
{
    return *_doc_id_mapping->find(d_id);
}

std::vector<doc_id> inverted_index::docs() const
{
    std::vector<doc_id> ret(_doc_id_mapping->size());
    std::iota(ret.begin(), ret.end(), 0);
    return ret;
}

void inverted_index::tokenize(corpus::document & doc)
{
    _tokenizer->tokenize(doc);
}

auto inverted_index::search_primary(term_id t_id) const
    -> std::shared_ptr<postings_data_type>
{
    uint64_t idx{t_id};

    // if the term doesn't exist in the index, return an empty postings_data
    if(idx >= _term_bit_locations->size())
        return std::make_shared<postings_data_type>(t_id);

    io::compressed_file_reader reader{*_postings,
        common::default_compression_reader_func};
    reader.seek(_term_bit_locations->at(idx));

    auto pdata = std::make_shared<postings_data_type>(t_id);
    pdata->read_compressed(reader);

    return pdata;
}

void inverted_index::chunk_handler::operator()(const corpus::document & doc)
{
    auto time = common::time([&]() {
        for(const auto & count: doc.counts())   // count: (string, double)
        {
            auto time = common::time<std::chrono::microseconds>([&]() {
                index_pdata_type pd{count.first};
                pd.increase_count(doc.id(), count.second);
                auto it = pdata_.find(pd);
                if(it == pdata_.end())
                {
                    chunk_size_ += pd.bytes_used();
                    pdata_.emplace(pd);
                }
                else
                {
                    chunk_size_ -= it->bytes_used();

                    // note: we can modify elements in this set because we do not change
                    // how comparisons are made (the primary_key value)
                    auto time = common::time<std::chrono::microseconds>([&]() {
                        //const_cast<postings_data_type &>(*it).merge_with(pd);
                        const_cast<index_pdata_type &>(*it).increase_count(doc.id(), count.second);
                    });
                    merging_with_time_ += time.count();
                    chunk_size_ += it->bytes_used();
                }
            });
            merging_pdata_time_ += time.count();

            time = common::time<std::chrono::microseconds>([&]() {
                if(chunk_size_ >= max_size)
                    flush_chunk();
            });
            writing_chunk_time_ += time.count();
        }
    });
    total_time_ += time.count();
}

void inverted_index::chunk_handler::flush_chunk() {
    if (chunk_size_ == 0)
        return;

    std::vector<index_pdata_type> pdata;
    for (auto it = pdata_.begin(); it != pdata_.end(); it = pdata_.erase(it))
        pdata.emplace_back(std::move(*it));
    pdata_.clear();
    std::sort(pdata.begin(), pdata.end());
    idx_->write_chunk(chunk_num_.fetch_add(1), pdata);
    chunk_size_ = 0;
}

inverted_index::chunk_handler::~chunk_handler() {
    flush_chunk();
}

}
}
