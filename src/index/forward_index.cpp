/**
 * @file forward_index.cpp
 * @author Sean Massung
 */

#include "index/forward_index.h"
#include "index/inverted_index.h"
#include "index/postings_data.h"
#include "index/string_list.h"
#include "index/string_list_writer.h"
#include "index/vocabulary_map.h"
#include "io/libsvm_parser.h"
#include "parallel/thread_pool.h"
#include "tokenizers/tokenizer.h"
#include "util/disk_vector.h"
#include "util/mapping.h"
#include "util/shim.h"

namespace meta {
namespace index {

forward_index::forward_index(const cpptoml::toml_group & config):
    disk_index{config},
    _index_name{*cpptoml::get_as<std::string>(config, "forward-index")}
{ /* nothing */ }

std::string forward_index::index_name() const
{
    return _index_name;
}

std::string forward_index::liblinear_data(doc_id d_id) const
{
    if(d_id >= num_docs())
        throw forward_index_exception{"invalid doc_id in search_primary"};

    uint64_t begin = (*_doc_byte_locations)[d_id];
    uint64_t length = 0;
    while(_postings->start()[begin + length] != '\n')
    {
        ++length; // TODO maybe save lengths as well?
        if(begin + length >= _postings->size())
            throw forward_index_exception{"out of bounds!"};
    }

    return std::string{_postings->start() + begin, length};
}

void forward_index::load_index()
{
    LOG(info) << "Loading index from disk: " << _index_name << ENDLG;

    init_metadata();

    _doc_id_mapping = make_unique<string_list>(_index_name + "/docids.mapping");
    _postings = make_unique<io::mmap_file>(
        _index_name + "/postings.index"
    );

    auto config = cpptoml::parse_file(_index_name + "/config.toml");
    if(is_libsvm_format(config))
        _term_id_mapping = nullptr;
    else
    {
        _term_id_mapping =
            make_unique<vocabulary_map>(_index_name + "/termids.mapping");
    }

    map::load_mapping(_label_ids, _index_name + "/labelids.mapping");
    _tokenizer = tokenizers::tokenizer::load(config);
}

void forward_index::create_index(const std::string & config_file)
{
    filesystem::copy_file(config_file, _index_name + "/config.toml");
    auto config = cpptoml::parse_file(_index_name + "/config.toml");
    _tokenizer = tokenizers::tokenizer::load(config);

    // if the corpus is a single libsvm formatted file, then we are done;
    // otherwise, we will create an inverted index and the uninvert it
    if(is_libsvm_format(config))
    {
        LOG(info) << "Creating index from libsvm data: " << _index_name << ENDLG;

        create_libsvm_postings(config);
        create_libsvm_metadata(config);
        _term_id_mapping = nullptr; // we don't know what the terms are!
        map::save_mapping(_label_ids, _index_name + "/labelids.mapping");
    }
    else
    {
        LOG(info) << "Creating index by uninverting: " << _index_name << ENDLG;
        make_index<inverted_index, caching::default_dblru_cache>(config_file,
                uint64_t{100000});

        create_uninverted_metadata(config);
        map::load_mapping(_label_ids, _index_name + "/labelids.mapping");
        uninvert(config);
        init_metadata();
        _postings = make_unique<io::mmap_file>(_index_name + "/postings.index");
        set_doc_byte_locations();
    }

    // now that the files are tokenized, we can create the string_list
    _doc_id_mapping = make_unique<string_list>(_index_name + "/docids.mapping");

    LOG(info) << "Done creating index: " << _index_name << ENDLG;
}

void forward_index::create_libsvm_postings(const cpptoml::toml_group& config)
{
    auto prefix = config.get_as<std::string>("prefix");
    if (!prefix)
        throw forward_index_exception{"prefix missing from configuration file"};

    auto dataset = config.get_as<std::string>("dataset");
    if (!dataset)
        throw forward_index_exception{"dataset missing from configuration file"};

    std::string existing_file = *prefix + "/"
        + *dataset + "/" + *dataset + ".dat";

    filesystem::copy_file(existing_file, _index_name + "/postings.index");

    init_metadata();

    // now, assign byte locations for libsvm doc starting points
    _postings = make_unique<io::mmap_file>(_index_name + "/postings.index");
    set_doc_byte_locations();
}

void forward_index::set_doc_byte_locations()
{
    doc_id d_id{0};
    uint8_t last_byte = '\n';
    for(uint64_t idx = 0; idx < _postings->size(); ++idx)
    {
        if(last_byte == '\n')
        {
            (*_doc_byte_locations)[d_id] = idx;
            ++d_id;
        }
        last_byte = _postings->start()[idx];
    }
}

void forward_index::init_metadata()
{
    uint64_t num_docs = filesystem::num_lines(_index_name + "/postings.index");
    _doc_sizes = make_unique<util::disk_vector<double>>(
        _index_name + "/docsizes.counts", num_docs);
    _labels = make_unique<util::disk_vector<label_id>>(
        _index_name + "/docs.labels", num_docs);
    _unique_terms = make_unique<util::disk_vector<uint64_t>>(
        _index_name + "/docs.uniqueterms", num_docs);
    _doc_byte_locations = make_unique<util::disk_vector<uint64_t>>(
        _index_name + "/lexicon.index", num_docs);
}

void forward_index::create_libsvm_metadata(const cpptoml::toml_group& config)
{
    doc_id d_id{0};
    std::unordered_set<term_id> terms;

    std::ifstream in{_index_name + "/postings.index"};
    std::string line;
    string_list_writer doc_id_mapping{_index_name + "/docids.mapping", num_docs()};
    while(in.good())
    {
        std::getline(in, line);
        if(line.empty())
            break;

        class_label lbl = io::libsvm_parser::label(line);
        (*_labels)[d_id] = get_label_id(lbl);

        uint64_t num_unique = 0;
        uint64_t length = 0;
        for(auto & count_pair: io::libsvm_parser::counts(line))
        {
            ++num_unique;
            terms.insert(count_pair.first);
            length += static_cast<uint64_t>(count_pair.second); // TODO
        }

        doc_id_mapping.insert(d_id, "[no path]");
        (*_doc_sizes)[d_id] = length;
        (*_unique_terms)[d_id] = num_unique;

        ++d_id;
    }

    _total_unique_terms = terms.size();
}

void forward_index::create_uninverted_metadata(const cpptoml::toml_group& config)
{
    auto files = { "/docids.mapping", "/docids.mapping_index.vector",
        "/termids.mapping", "/termids.mapping.inverse.vector",
        "/docsizes.counts.vector", "/docs.labels.vector",
        "/docs.uniqueterms.vector", "/labelids.mapping" };

    auto inv_name = *cpptoml::get_as<std::string>(config, "inverted-index");
    for(auto & file: files)
        filesystem::copy_file(inv_name + file, _index_name + file);
}

bool forward_index::is_libsvm_format(const cpptoml::toml_group& config) const
{
    auto tokenizers = config.get_group_array("tokenizers")->array();
    if(tokenizers.size() != 1)
        return false;

    auto method = tokenizers[0]->get_as<std::string>("method");
    if(!method)
        throw forward_index_exception{"failed to find tokenizer method"};

    return *method == "libsvm";
}

uint64_t forward_index::unique_terms() const
{
    if(_term_id_mapping == nullptr)
        return _total_unique_terms;

    return _term_id_mapping->size();
}

auto forward_index::search_primary(doc_id d_id) const
    -> std::shared_ptr<postings_data_type>
{
    auto pdata = std::make_shared<postings_data_type>(d_id);
    std::string line{liblinear_data(d_id)};
    pdata->set_counts(io::libsvm_parser::counts(line));
    return pdata;
}

void forward_index::uninvert(const cpptoml::toml_group& config)
{
    auto inv_name = *cpptoml::get_as<std::string>(config, "inverted-index");
    io::compressed_file_reader inv_reader{inv_name + "/postings.index",
                                          io::default_compression_reader_func};
    term_id t_id{0};
    std::atomic<uint32_t> chunk_num{0};
    {
        chunk_handler handler{this, chunk_num};
        while(inv_reader.has_next())
        {
            inverted_pdata_type pdata{t_id};
            pdata.read_compressed(inv_reader);
            handler(pdata);
            ++t_id;
        }
    }

    merge_chunks(chunk_num.load());
}

void forward_index::merge_chunks(uint32_t num_chunks)
{
    // here we just use the chunk shell for simplicity
    using chunk_t = chunk<primary_key_type, secondary_key_type>;
    std::priority_queue<chunk_t> chunks;
    for(uint32_t i = 0; i < num_chunks; ++i)
        chunks.emplace(_index_name + "/chunk-" + std::to_string(i));

    size_t remaining = chunks.size() - 1;
    std::mutex mutex;
    auto task = [&]() {
        while (true) {
            util::optional<chunk_t> first;
            util::optional<chunk_t> second;
            {
                std::lock_guard<std::mutex> lock{mutex};
                if (chunks.size() < 2)
                    return;
                first = util::optional<chunk_t>{chunks.top()};
                chunks.pop();
                second = util::optional<chunk_t>{chunks.top()};
                chunks.pop();
                LOG(progress) << "> Merging " << first->path() << " ("
                    << printing::bytes_to_units(first->size())
                    << ") and " << second->path() << " ("
                    << printing::bytes_to_units(second->size())
                    << "), " << --remaining << " remaining        \r" << ENDLG;
            }
            first->merge_with(*second);
            {
                std::lock_guard<std::mutex> lock{mutex};
                chunks.push(*first);
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
    doc_ids_to_labels(chunks.top().path());
}

void forward_index::doc_ids_to_labels(const std::string & filename)
{
    _labels = make_unique<util::disk_vector<label_id>>(_index_name + "/docs.labels");

    std::ofstream output{_index_name + "/postings.index"};
    io::compressed_file_reader input{filename,
        io::default_compression_reader_func};

    // read from input, write to output, changing doc_id to class_label for the
    // correct libsvm format
    index_pdata_type pdata;
    while(input >> pdata)
    {
        doc_id d_id = pdata.primary_key();
        pdata.set_primary_key(static_cast<doc_id>((*_labels)[d_id]));
        pdata.write_libsvm(output);
    }

    filesystem::delete_file(filename);
}

void forward_index::chunk_handler::flush_chunk()
{
    if (chunk_size_ == 0)
        return;

    std::vector<index_pdata_type> pdata;
    for (auto it = pdata_.begin(); it != pdata_.end(); it = pdata_.erase(it))
        pdata.emplace_back(std::move(*it));
    pdata_.clear();
    std::sort(pdata.begin(), pdata.end());

    std::string chunk_name = idx_->_index_name + "/chunk-" + std::to_string(chunk_num_);
    io::compressed_file_writer outfile{chunk_name,
        io::default_compression_writer_func};
    for(auto & p: pdata)
        outfile << p;

    pdata.clear();
    chunk_size_ = 0;
    chunk_num_.fetch_add(1);
}

void forward_index::chunk_handler::operator()(const inverted_pdata_type & single_pdata)
{
    for(const auto& count: single_pdata.counts()) // count: (doc_id, uint)
    {
        index_pdata_type pd{count.first};
        pd.increase_count(single_pdata.primary_key(), count.second);
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
            const_cast<index_pdata_type&>(*it).increase_count(
                    single_pdata.primary_key(), count.second);
            chunk_size_ += it->bytes_used();
        }

        if(chunk_size_ >= max_size)
            flush_chunk();
    }
}

forward_index::chunk_handler::~chunk_handler()
{
    flush_chunk();
}

}
}
