/**
 * @file inverted_index.h
 * @author Sean Massung
 */

#include <sys/stat.h>
#include <cstdio>
#include <queue>
#include <iostream>
#include "index/inverted_index.h"
#include "index/chunk.h"

#define USE_CACHE false

using std::cerr;
using std::endl;

namespace meta {
namespace index {

inverted_index::inverted_index(const std::string & index_name,
                               std::vector<document> & docs,
                               std::shared_ptr<tokenizers::tokenizer> & tok):
    _index_name(index_name),
    _cache(util::splay_cache<term_id, postings_data>{10})
{
    if(mkdir(_index_name.c_str(), 0755) == -1)
        throw inverted_index_exception{"directory already exists"};

    uint32_t num_chunks = tokenize_docs(docs, tok);
    merge_chunks(num_chunks, index_name + "/postings.index");
    create_lexicon(index_name + "/postings.index", index_name + "/lexicon.index");
    tok->save_term_id_mapping(index_name + "/termids.mapping");
    save_mapping(_doc_id_mapping, index_name + "/docids.mapping");
    save_mapping(_doc_sizes, index_name + "/docsizes.counts");

    _postings = std::unique_ptr<io::mmap_file>{
        new io::mmap_file{index_name + "/postings.index"}
    };
}

template <class Key, class Value>
void inverted_index::save_mapping(const std::unordered_map<Key, Value> & map,
                                  const std::string & filename) const
{
    std::ofstream outfile{filename};
    for(auto & p: map)
        outfile << p.first << " " << p.second << "\n";
    outfile.close();
}

uint32_t inverted_index::tokenize_docs(std::vector<document> & docs,
                                   std::shared_ptr<tokenizers::tokenizer> & tok)
{
    std::unordered_map<term_id, postings_data> pdata;
    uint32_t chunk_num = 0;
    uint64_t doc_num = 0;
    for(auto & doc: docs)
    {
        cerr << "[II] Tokenizing " << doc.name() << endl;
        tok->tokenize(doc);
        _doc_id_mapping[doc_num] = doc.name();
        _doc_sizes[doc_num] = doc.length();

        for(auto & f: doc.frequencies())
        {
            postings_data pd{f.first};
            pd.increase_count(doc_num, f.second);
            auto it = pdata.find(f.first);
            if(it == pdata.end())
                pdata.insert(std::make_pair(f.first, pd));
            else
                it->second.merge_with(pd);
        }

        ++doc_num;

        // every k documents, write a chunk
        // TODO: make this based on memory usage instead
        if(doc_num % 100 == 0)
            write_chunk(chunk_num++, pdata);
    }
    
    if(!pdata.empty())
        write_chunk(chunk_num++, pdata);

    return chunk_num;
}

void inverted_index::write_chunk(uint32_t chunk_num,
                                 std::unordered_map<term_id, postings_data> & pdata)
{
    cerr << "[II] Writing chunk " << chunk_num << endl;

    std::vector<std::pair<term_id, postings_data>> sorted{pdata.begin(), pdata.end()};
    std::sort(sorted.begin(), sorted.end());

    std::ofstream outfile{"chunk-" + common::to_string(chunk_num)};
    for(auto & p: sorted)
        outfile << p.second;
    outfile.close();

    pdata.clear();
}

void inverted_index::merge_chunks(uint32_t num_chunks, const std::string & filename)
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

        cerr << "[II] Merging " << first.path() << " (" << first.size()
             << " bytes) and " << second.path() << " (" << second.size()
             << " bytes)" << endl;

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

    cerr << "[II] Finished creating postings file " << filename
         << " (" << size << " " << units << ")" << endl;
}

void inverted_index::create_lexicon(const std::string & postings_file,
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

inverted_index::inverted_index(const std::string & index_path):
    _cache(util::splay_cache<term_id, postings_data>{10})
{
    load_mapping(_doc_id_mapping, index_path + "/docids.mapping");
    load_mapping(_doc_sizes, index_path + "/docsizes.counts");
    load_mapping(_term_locations, index_path + "/lexicon.index");
    // load termid -> term info? will I need this?
    _postings = std::unique_ptr<io::mmap_file>{
        new io::mmap_file{_index_name + "/postings.index"}
    };
}

template <class Key, class Value>
void inverted_index::load_mapping(std::unordered_map<Key, Value> & map,
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

uint64_t inverted_index::idf(term_id t_id)
{
    postings_data pdata = search_term(t_id);
    return pdata.idf();
}

uint64_t inverted_index::doc_size(doc_id d_id) const
{
    auto it = _doc_sizes.find(d_id);
    if(it == _doc_sizes.end())
        throw inverted_index_exception{"nonexistent doc id"};
    return it->second;
}

uint64_t inverted_index::term_freq(term_id t_id, doc_id d_id)
{
    postings_data pdata = search_term(t_id);
    return pdata.count(d_id);
}

postings_data inverted_index::search_term(term_id t_id)
{
#if USE_CACHE
    if(_cache.exists(t_id))
        return _cache.find(t_id);
#endif

    auto it = _term_locations.find(t_id);
    if(it == _term_locations.end())
        throw inverted_index_exception{"term does not exist in index"};

    uint64_t idx = it->second;
    postings_data pdata = search_postings(idx);

#if USE_CACHE
    cache.insert(t_id, pdata);
#endif

    return pdata;
}

postings_data inverted_index::search_postings(uint64_t idx)
{
    uint64_t len = 0;
    char* post = _postings->start();

    while(post[idx + len] != '\n')
        ++len;

    std::string raw{post + idx, len};
    return postings_data{raw};
}

}
}
