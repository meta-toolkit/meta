/**
 * @file disk_index.tcc
 * @author Sean Massung
 */

#include <cstdio>
#include <numeric>
#include <iostream>
#include <utility>
#include <sys/stat.h>
#include "index/disk_index.h"
#include "logging/logger.h"
#include "parallel/thread_pool.h"
#include "util/common.h"
#include "util/optional.h"

namespace meta {
namespace index {

template <class Index, class... Args>
Index make_index(const std::string & config_file, Args &&... args)
{
    auto config = cpptoml::parse_file(config_file);

    // check if we have paths specified for either kind of index
    if (!(config.contains("forward-index")
          && config.contains("inverted-index"))) {
        throw typename Index::disk_index_exception{
            "forward-index or inverted-index missing from configuration file"
        };
    }

    Index idx{config, std::forward<Args>(args)...};

    // if index has already been made, load it
    if(mkdir(idx._index_name.c_str(), 0755) == -1)
        idx.load_index();
    else
        idx.create_index(config_file);
    return idx;
}

template <class Index, template <class, class> class Cache, class... Args>
cached_index<Index, Cache> make_index(const std::string & config_file,
                                      Args &&... args) {
    return make_index<cached_index<Index, Cache>>(config_file,
                                                  std::forward<Args>(args)...);
}

template <class DerivedIndex>
disk_index<DerivedIndex>::disk_index(const cpptoml::toml_group & config,
                                     const std::string & index_path):
    _tokenizer{ tokenizers::tokenizer::load(config) },
    _index_name{ index_path }
{ /* nothing */ }

template <class DerivedIndex>
std::string disk_index<DerivedIndex>::index_name() const
{
    return _index_name;
}

template <class DerivedIndex>
uint64_t disk_index<DerivedIndex>::unique_terms(doc_id d_id) const
{
    return _unique_terms->at(d_id);
}

template <class DerivedIndex>
uint64_t disk_index<DerivedIndex>::unique_terms() const
{
    return _term_id_mapping->size();
}

template <class DerivedIndex>
void disk_index<DerivedIndex>::create_index(const std::string & config_file)
{
    // save the config file so we can recreate the tokenizer
    std::ifstream source_config{config_file.c_str(), std::ios::binary};
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

    time = common::time([&](){
        merge_chunks(_index_name + "/postings.index");
    });

    LOG(debug) << " ! Time spent merging: " << (time.count() / 1000.0) << ENDLG;

    time = common::time([&](){
        compress(_index_name + "/postings.index");
    });

    LOG(debug) << " ! Time spent compressing: " << (time.count() / 1000.0) << ENDLG;

    common::save_mapping(_label_ids, _index_name + "/labelids.mapping");

    _postings = common::make_unique<io::mmap_file>(_index_name + "/postings.index");

    LOG(info) << "Done creating index: " << _index_name << ENDLG;
}

template <class DerivedIndex>
uint32_t disk_index<DerivedIndex>::tokenize_docs(corpus::corpus * docs)
{
    std::mutex mutex;
    std::atomic<uint32_t> chunk_num{0};
    std::atomic<uint64_t> token_time{0};
    std::atomic<uint64_t> metadata_time{0};
    std::atomic<uint64_t> handler_time{0};
    uint64_t loading_docs_time{0};

    auto task = [&]() {
        typename DerivedIndex::chunk_handler handler{this, chunk_num};
        while (true) {
            util::optional<corpus::document> doc;
            {
                std::lock_guard<std::mutex> lock{mutex};

                if (!docs->has_next()) {
                    handler.print_stats();
                    return; // destructor for handler will write
                            // any intermediate chunks
                }
                auto time = common::time([&]() {
                    doc = docs->next();
                });
                loading_docs_time += time.count();

                std::string progress = "> Documents: "
                    + common::add_commas(common::to_string(doc->id()))
                    + " Unique primary keys: "
                    + common::add_commas(
                        common::to_string(_term_id_mapping->size()))
                    + " Tokenizing: ";
                common::show_progress(doc->id(), docs->size(), 100, progress);
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
        + common::add_commas(common::to_string(docs->size()))
        + " Unique primary keys: "
        + common::add_commas(common::to_string(_term_id_mapping->size()))
        + " Tokenizing: ";
    common::end_progress(progress);

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

template <class DerivedIndex>
void disk_index<DerivedIndex>::compress(const std::string & filename)
{
    std::string cfilename{filename + ".compressed"};

    // allocate memory for the term_id -> term location mapping now that we know
    // how many terms there are
    _term_bit_locations = common::make_unique<util::disk_vector<uint64_t>>(
            _index_name + "/lexicon.index", _term_id_mapping->size());

    // create scope so the writer closes and we can calculate the size of the
    // file as well as rename it
    {
        io::compressed_file_writer out{cfilename, [&](uint64_t key) {
            if(key == std::numeric_limits<uint64_t>::max()) // delimiter
                return uint64_t{1};
            return key + 2;
        }};

        /*
        postings_data_type pdata{primary_key_type{0}};
        */
        postings_data<std::string, doc_id> pdata; // TODO: breaks forward_index
        std::ifstream in{filename};

        in.seekg(0, in.end);
        auto length = in.tellg();
        in.seekg(0, in.beg);
        auto idx = in.tellg();

        uint64_t unique_terms;
        common::read_binary(in, unique_terms);

        // allocate memory for the term_id -> term location mapping now
        // that we know how many terms there are
        _term_bit_locations = common::make_unique<util::disk_vector<uint64_t>>(
                _index_name + "/lexicon.index", unique_terms);

        // note: we will be accessing pdata in sorted order
        while(in >> pdata)
        {
            if (idx != in.tellg() / (length / 500))
            {
                idx = in.tellg() / (length / 500);
                common::show_progress(idx, 500, 1,
                        " > Creating compressed postings file: ");
            }
            auto t_id = get_term_id(pdata.primary_key());
            (*_term_bit_locations)[t_id] = out.bit_location();
            pdata.write_compressed(out);
        }
        common::end_progress(" > Creating compressed postings file: ");
    }

    LOG(info) << "Created compressed postings file ("
          << common::bytes_to_units(common::file_size(cfilename))
          << ")" << ENDLG;

    remove(filename.c_str());
    rename(cfilename.c_str(), filename.c_str());
}

template <class DerivedIndex>
term_id disk_index<DerivedIndex>::get_term_id(const std::string & term)
{
    std::lock_guard<std::mutex> lock{*_mutex};

    auto termID = _term_id_mapping->find(term);
    if(termID)
        return term_id{*termID};

    uint64_t size = _term_id_mapping->size();
    _term_id_mapping->insert(term, term_id{size});
    return term_id{size};
}

template <class DerivedIndex>
void disk_index<DerivedIndex>::load_index()
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

template <class DerivedIndex>
class_label disk_index<DerivedIndex>::label(doc_id d_id) const
{
    return class_label_from_id(_labels->at(d_id));
}

template <class DerivedIndex>
class_label disk_index<DerivedIndex>::class_label_from_id(label_id l_id) const
{
    return _label_ids.get_key(l_id);
}

template <class DerivedIndex>
label_id disk_index<DerivedIndex>::get_label_id(const class_label & lbl)
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

template <class DerivedIndex>
label_id disk_index<DerivedIndex>::label_id_from_doc(doc_id d_id) const
{
    return _labels->at(d_id);
}

template <class DerivedIndex>
template <class Container>
void disk_index<DerivedIndex>::write_chunk(uint32_t chunk_num,
                                           Container & pdata)
{
    std::string chunk_name =
        _index_name + "/chunk-" + common::to_string(chunk_num);
    _chunks.emplace(chunk_name);
    std::ofstream outfile{chunk_name};

    common::write_binary(outfile, uint64_t{pdata.size()});
    for(auto & p: pdata)
        outfile << p;

    outfile.close();
    pdata.clear();
}

template <class DerivedIndex>
void disk_index<DerivedIndex>::merge_chunks(const std::string & filename)
{
    using chunk_t = chunk<std::string, secondary_key_type>;

    // this represents the number of merge steps needed---it is equivalent
    // to the number of internal nodes in a binary tree with n leaf nodes
    size_t remaining = _chunks.size() - 1;
    std::mutex mutex;
    parallel::thread_pool pool;
    auto thread_ids = pool.thread_ids();
    std::vector<std::future<void>> futures;
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
                    << common::bytes_to_units(first->size())
                    << ") and " << second->path() << " ("
                    << common::bytes_to_units(second->size())
                    << "), " << --remaining << " remaining        \r" << ENDLG;
            }
            first->merge_with(*second);
            {
                std::lock_guard<std::mutex> lock{mutex};
                _chunks.push(*first);
            }
        }
    };

    for (size_t i = 0; i < thread_ids.size(); ++i)
        futures.emplace_back(pool.submit_task(task));

    for (auto & fut : futures)
        fut.get();

    LOG(progress) << '\n' << ENDLG;

    rename(_chunks.top().path().c_str(), filename.c_str());

    LOG(info) << "Created uncompressed postings file " << filename
              << " (" << common::bytes_to_units(_chunks.top().size()) << ")"
              << ENDLG;
}

template <class DerivedIndex>
double disk_index<DerivedIndex>::doc_size(doc_id d_id) const
{
    return _doc_sizes->at(d_id);
}

template <class DerivedIndex>
uint64_t disk_index<DerivedIndex>::num_docs() const
{
    return _doc_sizes->size();
}

template <class DerivedIndex>
std::string disk_index<DerivedIndex>::doc_name(doc_id d_id) const
{
    auto path = doc_path(d_id);
    return path.substr(path.find_last_of("/") + 1);
}

template <class DerivedIndex>
std::string disk_index<DerivedIndex>::doc_path(doc_id d_id) const
{
    return *_doc_id_mapping->find(d_id);
}

template <class DerivedIndex>
std::vector<doc_id> disk_index<DerivedIndex>::docs() const
{
    std::vector<doc_id> ret(_doc_id_mapping->size());
    std::iota(ret.begin(), ret.end(), 0);
    return ret;
}

template <class DerivedIndex>
void disk_index<DerivedIndex>::tokenize(corpus::document & doc)
{
    _tokenizer->tokenize(doc);
}

template <class DerivedIndex>
auto disk_index<DerivedIndex>::search_primary(primary_key_type p_id) const
    -> std::shared_ptr<postings_data_type>
{
    uint64_t idx{p_id};

    // if the term doesn't exist in the index, return an empty postings_data
    if(idx >= _term_bit_locations->size())
        return std::make_shared<postings_data_type>(p_id);

    io::compressed_file_reader reader{*_postings, [&](uint64_t value) {
        if(value == 1)
            return std::numeric_limits<uint64_t>::max(); // delimiter
        return value - 2;
    }};
    reader.seek(_term_bit_locations->at(idx));

    auto pdata = std::make_shared<postings_data_type>(p_id);
    pdata->read_compressed(reader);

    return pdata;
}

}
}
