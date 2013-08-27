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
#include "io/config_reader.h"

#define USE_CACHE true

using std::cerr;
using std::endl;

namespace meta {
namespace index {

disk_index::disk_index(const std::string & index_name,
                               std::shared_ptr<tokenizers::tokenizer> & tok):
    _tokenizer(tok),
    _index_name(index_name),
    _cache(util::splay_cache<term_id, postings_data<term_id, doc_id>>{10})
{ /* nothing */ }

void disk_index::create_index(std::vector<document> & docs,
                              const std::string & config_file)
{
    if(mkdir(_index_name.c_str(), 0755) == -1)
        throw disk_index_exception{"directory already exists"};

    uint32_t num_chunks = tokenize_docs(docs);
    merge_chunks(num_chunks, _index_name + "/postings.index");
    create_lexicon(_index_name + "/postings.index", _index_name + "/lexicon.index");
    _tokenizer->save_term_id_mapping(_index_name + "/termids.mapping");
    save_mapping(_doc_id_mapping, _index_name + "/docids.mapping");
    save_mapping(_doc_sizes, _index_name + "/docsizes.counts");

    _postings = std::unique_ptr<io::mmap_file>{
        new io::mmap_file{_index_name + "/postings.index"}
    };

    // save the config file so we can recreate the tokenizer
    std::ifstream source_config{config_file.c_str(), std::ios::binary};
    std::ofstream dest_config{_index_name + "/config.toml", std::ios::binary};
    dest_config << source_config.rdbuf();
}

template <class Key, class Value>
void disk_index::save_mapping(const std::unordered_map<Key, Value> & map,
                                  const std::string & filename) const
{
    std::ofstream outfile{filename};
    for(auto & p: map)
        outfile << p.first << " " << p.second << "\n";
    outfile.close();
}

void disk_index::write_chunk(uint32_t chunk_num,
                                 std::unordered_map<term_id, postings_data<term_id, doc_id>> & pdata)
{
    std::vector<std::pair<term_id, postings_data<term_id, doc_id>>> sorted{pdata.begin(), pdata.end()};
    std::sort(sorted.begin(), sorted.end());

    std::ofstream outfile{"chunk-" + common::to_string(chunk_num)};
    for(auto & p: sorted)
        outfile << p.second;
    outfile.close();

    pdata.clear();
}

void disk_index::merge_chunks(uint32_t num_chunks, const std::string & filename)
{
    // create priority queue of all chunks based on size
    std::priority_queue<chunk> chunks;
    for(uint32_t i = 0; i < num_chunks; ++i)
    {
        std::string filename = "chunk-" + common::to_string(i);
        chunks.push(chunk{filename});
    }

    // merge the smallest two chunks together until there is only one left
    while(chunks.size() > 1)
    {
        chunk first = chunks.top();
        chunks.pop();
        chunk second = chunks.top();
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

void disk_index::create_lexicon(const std::string & postings_file,
                                    const std::string & lexicon_file)
{
    io::mmap_file pfile{postings_file};
    char* postings = pfile.start();
    term_id cur_id = 1;
    _term_locations[0] = 0;
    uint64_t idx = 0;
    while(idx < pfile.size() - 1)
    {
        if(postings[idx] == '\n')
            _term_locations[cur_id++] = idx + 1;
        ++idx;
    }
    save_mapping(_term_locations, lexicon_file);
}

disk_index::disk_index(const std::string & index_path):
    _index_name(index_path),
    _cache(util::splay_cache<term_id, postings_data<term_id, doc_id>>{10})
{
    load_mapping(_doc_id_mapping, index_path + "/docids.mapping");
    load_mapping(_doc_sizes, index_path + "/docsizes.counts");
    load_mapping(_term_locations, index_path + "/lexicon.index");

    _postings = std::unique_ptr<io::mmap_file>{
        new io::mmap_file{index_path + "/postings.index"}
    };

    auto config = io::config_reader::read(index_path + "/config.toml");
    _tokenizer = tokenizers::tokenizer::load_tokenizer(config);
    _tokenizer->set_term_id_mapping(index_path + "/termids.mapping");
}

template <class Key, class Value>
void disk_index::load_mapping(std::unordered_map<Key, Value> & map,
                                  const std::string & filename)
{
    std::ifstream input{filename};
    while(input.good())
    {
        Key k;
        Value v;
        input >> k;
        input >> v;
        map[k] = v;
    }
    input.close();
}

uint64_t disk_index::doc_size(doc_id d_id) const
{
    auto it = _doc_sizes.find(d_id);
    if(it == _doc_sizes.end())
        throw disk_index_exception{"nonexistent doc id"};
    return it->second;
}

uint64_t disk_index::num_docs() const
{
    return _doc_sizes.size();
}

std::string disk_index::doc_name(doc_id d_id) const
{
    return common::safe_at(_doc_id_mapping, d_id);
}

std::vector<doc_id> disk_index::docs() const
{
    std::vector<doc_id> ret;
    ret.reserve(_doc_id_mapping.size());
    for(auto & d: _doc_id_mapping)
        ret.push_back(d.first);
    return ret;
}

void disk_index::tokenize(document & doc)
{
    _tokenizer->tokenize(doc);
}

postings_data<term_id, doc_id> disk_index::search_term(term_id t_id) const
{
#if USE_CACHE
    {
        std::lock_guard<std::mutex> lock{_mutex};
        if(_cache.exists(t_id))
            return _cache.find(t_id);
    }
#endif

    auto it = _term_locations.find(t_id);
    // if the term doesn't exist in the index, return an empty postings_data
    if(it == _term_locations.end())
        return postings_data<term_id, doc_id>{t_id};

    uint64_t idx = it->second;
    postings_data<term_id, doc_id> pdata = search_postings(idx);

#if USE_CACHE
    {
        std::lock_guard<std::mutex> lock{_mutex};
        _cache.insert(t_id, pdata);
    }
#endif

    return pdata;
}

postings_data<term_id, doc_id> disk_index::search_postings(uint64_t idx) const
{
    uint64_t len = 0;
    char* post = _postings->start();

    while(post[idx + len] != '\n')
        ++len;

    std::string raw{post + idx, len};
    return postings_data<term_id, doc_id>{raw};
}

}
}
