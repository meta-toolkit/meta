/**
 * @file disk_index.cpp
 * @author Sean Massung
 */

#include <sys/stat.h>
#include <cstdio>
#include <queue>
#include <iostream>
#include "index/disk_index.h"
#include "index/chunk.h"
#include "util/common.h"

#define USE_CACHE true

using std::cerr;
using std::endl;

namespace meta {
namespace index {

template <class Index>
Index make_index(const std::string & config_file)
{
    auto config = common::read_config(config_file);
    Index idx{config};
    if(mkdir(idx._index_name.c_str(), 0755) == -1) 
    {
        // index has already been made, load it
        idx.load_index();
    }
    else
    {
        // otherwise, create a new one
        std::string prefix = *cpptoml::get_as<std::string>(config, "prefix")
            + *cpptoml::get_as<std::string>(config, "dataset");
        std::string corpus_file = prefix 
            + "/" 
            + *cpptoml::get_as<std::string>(config, "list")
            + "-full-corpus.txt";

        auto docs = index::document::load_docs(corpus_file, prefix);
        idx.create_index(docs, config_file);
    }
    return idx;
}

template <class PrimaryKey, class SecondaryKey>
disk_index<PrimaryKey, SecondaryKey>::disk_index(
        const cpptoml::toml_group & config,
        const std::string & index_path):
    _tokenizer{ tokenizers::tokenizer::load_tokenizer(config) },
    _index_name{ index_path },
    _cache{10}, 
    _mutex{ new std::mutex() }
{ /* nothing */ }

template <class PrimaryKey, class SecondaryKey>
std::string disk_index<PrimaryKey, SecondaryKey>::index_name() const
{
    return _index_name;
}

template <class PrimaryKey, class SecondaryKey>
void disk_index<PrimaryKey, SecondaryKey>::create_index(
        std::vector<document> & docs,
        const std::string & config_file)
{
    uint32_t num_chunks = tokenize_docs(docs);
    merge_chunks(num_chunks, _index_name + "/postings.index");
    create_lexicon(_index_name
            + "/postings.index", _index_name + "/lexicon.index");
    _tokenizer->save_term_id_mapping(_index_name + "/termids.mapping");
    save_mapping(_doc_id_mapping, _index_name + "/docids.mapping");
    save_mapping(_doc_sizes, _index_name + "/docsizes.counts");

    _postings = std::unique_ptr<io::mmap_file>{
        new io::mmap_file{_index_name + "/postings.index"}
    };

    // Save class label information; this is needed for both forward and
    // inverted indexes. It's assumed that doc_ids are assigned in deriving
    // classes in the same order the documents appear in the vector
    doc_id doc_num{0};
    for(auto & d: docs)
        if(d.label() != class_label{""})
            _labels[doc_num++] = d.label();
    save_mapping(_labels, _index_name + "/docs.labels");
    set_label_ids();

    // save the config file so we can recreate the tokenizer
    std::ifstream source_config{config_file.c_str(), std::ios::binary};
    std::ofstream dest_config{_index_name + "/config.toml", std::ios::binary};
    dest_config << source_config.rdbuf();
}

template <class PrimaryKey, class SecondaryKey>
void disk_index<PrimaryKey, SecondaryKey>::load_index()
{
    cerr << "Loading inverted index from disk ("
         << _index_name << ")..." << endl;

    load_mapping(_doc_id_mapping, _index_name + "/docids.mapping");
    load_mapping(_doc_sizes, _index_name + "/docsizes.counts");
    load_mapping(_term_locations, _index_name + "/lexicon.index");
    load_mapping(_labels, _index_name + "/docs.labels");
    set_label_ids();

    _postings = std::unique_ptr<io::mmap_file>{
        new io::mmap_file{_index_name + "/postings.index"}
    };

    auto config = common::read_config(_index_name + "/config.toml");
    _tokenizer = tokenizers::tokenizer::load_tokenizer(config);
    _tokenizer->set_term_id_mapping(_index_name + "/termids.mapping");
}

template <class PrimaryKey, class SecondaryKey>
class_label disk_index<PrimaryKey, SecondaryKey>::label(doc_id d_id) const
{
    return common::safe_at(_labels, d_id);
}

template <class PrimaryKey, class SecondaryKey>
class_label
disk_index<PrimaryKey, SecondaryKey>::class_label_from_id(label_id l_id) const
{
    return _label_ids.get_key(l_id);
}

template <class PrimaryKey, class SecondaryKey>
void disk_index<PrimaryKey, SecondaryKey>::set_label_ids()
{
    std::unordered_set<class_label> labels;
    for(auto & p: _labels)
        labels.insert(p.second);

    label_id i{0};
    for(auto & lbl: labels)
        _label_ids.insert(lbl, i++);
}

template <class PrimaryKey, class SecondaryKey>
label_id
disk_index<PrimaryKey, SecondaryKey>::label_id_from_doc(doc_id d_id) const
{
    return _label_ids.get_value(_labels.at(d_id));
}

template <class PrimaryKey, class SecondaryKey>
template <class Key, class Value>
void disk_index<PrimaryKey, SecondaryKey>::save_mapping(
        const std::unordered_map<Key, Value> & map,
        const std::string & filename) const
{
    std::ofstream outfile{filename};
    for(auto & p: map)
        outfile << p.first << " " << p.second << "\n";
    outfile.close();
}

template <class PrimaryKey, class SecondaryKey>
void disk_index<PrimaryKey, SecondaryKey>::write_chunk(
        uint32_t chunk_num,
        std::unordered_map<
            PrimaryKey,
            postings_data<PrimaryKey, SecondaryKey>
        > & pdata)
{
    std::vector<
        std::pair<PrimaryKey, postings_data<PrimaryKey, SecondaryKey>>
    > sorted{pdata.begin(), pdata.end()};
    std::sort(sorted.begin(), sorted.end());

    std::ofstream outfile{"chunk-" + common::to_string(chunk_num)};
    for(auto & p: sorted)
        outfile << p.second;
    outfile.close();

    pdata.clear();
}

template <class PrimaryKey, class SecondaryKey>
void disk_index<PrimaryKey, SecondaryKey>::merge_chunks(
        uint32_t num_chunks,
        const std::string & filename)
{
    // create priority queue of all chunks based on size
    std::priority_queue<chunk<PrimaryKey, SecondaryKey>> chunks;
    for(uint32_t i = 0; i < num_chunks; ++i)
    {
        std::string filename = "chunk-" + common::to_string(i);
        chunks.push(chunk<PrimaryKey, SecondaryKey>{filename});
    }

    // merge the smallest two chunks together until there is only one left
    while(chunks.size() > 1)
    {
        chunk<PrimaryKey, SecondaryKey> first = chunks.top();
        chunks.pop();
        chunk<PrimaryKey, SecondaryKey> second = chunks.top();
        chunks.pop();

        cerr << " Merging " << first.path() << " (" << first.size()
             << " bytes) and " << second.path() << " (" << second.size()
             << " bytes), " << chunks.size() << " remaining" << endl;

        first.merge_with(second);
        chunks.push(first);
    }

    rename(chunks.top().path().c_str(), filename.c_str());
    double size = chunks.top().size();
    std::string units = "bytes";

    for(auto & u: {"KB", "MB", "GB", "TB"})
    {
        if(size >= 1024)
        {
            size /= 1024;
            units = u;
        }
    }

    cerr << "Created postings file " << filename
         << " (" << size << " " << units << ")" << endl;
}

template <class PrimaryKey, class SecondaryKey>
void disk_index<PrimaryKey, SecondaryKey>::create_lexicon(
        const std::string & postings_file,
        const std::string & lexicon_file)
{
    io::mmap_file pfile{postings_file};
    char* postings = pfile.start();
    PrimaryKey cur_id{ 1 };
    _term_locations[PrimaryKey{0}] = 0;
    uint64_t idx = 0;
    while(idx < pfile.size() - 1)
    {
        if(postings[idx] == '\n')
            _term_locations[cur_id++] = idx + 1;
        ++idx;
    }
    save_mapping(_term_locations, lexicon_file);
}

template <class PrimaryKey, class SecondaryKey>
template <class Key, class Value>
void disk_index<PrimaryKey, SecondaryKey>::load_mapping(
        std::unordered_map<Key, Value> & map,
        const std::string & filename)
{
    std::ifstream input{filename};
    Key k;
    Value v;
    while((input >> k) && (input >> v))
        map[k] = v;
}

template <class PrimaryKey, class SecondaryKey>
uint64_t disk_index<PrimaryKey, SecondaryKey>::doc_size(doc_id d_id) const
{
    auto it = _doc_sizes.find(d_id);
    if(it == _doc_sizes.end())
        throw disk_index_exception{"nonexistent doc id"};
    return it->second;
}

template <class PrimaryKey, class SecondaryKey>
uint64_t disk_index<PrimaryKey, SecondaryKey>::num_docs() const
{
    return _doc_sizes.size();
}

template <class PrimaryKey, class SecondaryKey>
std::string disk_index<PrimaryKey, SecondaryKey>::doc_name(doc_id d_id) const
{
    auto path = doc_path(d_id);
    return path.substr(path.find_last_of("/") + 1);
}

template <class PrimaryKey, class SecondaryKey>
std::string disk_index<PrimaryKey, SecondaryKey>::doc_path(doc_id d_id) const
{
    return _doc_id_mapping.at(d_id);
}

template <class PrimaryKey, class SecondaryKey>
std::vector<doc_id> disk_index<PrimaryKey, SecondaryKey>::docs() const
{
    std::vector<doc_id> ret;
    ret.reserve(_doc_id_mapping.size());
    for(auto & d: _doc_id_mapping)
        ret.push_back(d.first);
    return ret;
}

template <class PrimaryKey, class SecondaryKey>
void disk_index<PrimaryKey, SecondaryKey>::tokenize(document & doc)
{
    _tokenizer->tokenize(doc);
}

template <class PrimaryKey, class SecondaryKey>
postings_data<PrimaryKey, SecondaryKey>
disk_index<PrimaryKey, SecondaryKey>::search_primary(PrimaryKey t_id) const
{
#if USE_CACHE
    {
        std::lock_guard<std::mutex> lock{*_mutex};
        if(_cache.exists(t_id))
            return _cache.find(t_id);
    }
#endif

    auto it = _term_locations.find(t_id);
    // if the term doesn't exist in the index, return an empty postings_data
    if(it == _term_locations.end())
        return postings_data<PrimaryKey, SecondaryKey>{t_id};

    uint64_t idx = it->second;
    postings_data<PrimaryKey, SecondaryKey> pdata = search_postings(idx);

#if USE_CACHE
    {
        std::lock_guard<std::mutex> lock{*_mutex};
        _cache.insert(t_id, pdata);
    }
#endif

    return pdata;
}

template <class PrimaryKey, class SecondaryKey>
postings_data<PrimaryKey, SecondaryKey>
disk_index<PrimaryKey, SecondaryKey>::search_postings(uint64_t idx) const
{
    uint64_t len = 0;
    char* post = _postings->start();

    while(post[idx + len] != '\n')
        ++len;

    std::string raw{post + idx, len};
    return postings_data<PrimaryKey, SecondaryKey>{raw};
}

}
}
