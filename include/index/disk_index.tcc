/**
 * @file disk_index.tcc
 * @author Sean Massung
 */

#include <sys/stat.h>
#include <cstdio>
#include <queue>
#include <iostream>
#include <utility>
#include "index/disk_index.h"
#include "index/chunk.h"
#include "util/common.h"

namespace meta {
namespace index {

template <class Index, class... Args>
Index make_index(const std::string & config_file, Args &&... args)
{
    auto config = cpptoml::parse_file(config_file);
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

template <class PrimaryKey, class SecondaryKey>
disk_index<PrimaryKey, SecondaryKey>::disk_index(
        const cpptoml::toml_group & config,
        const std::string & index_path):
    _tokenizer{ tokenizers::tokenizer::load_tokenizer(config) },
    _index_name{ index_path }
{ /* nothing */ }

template <class PrimaryKey, class SecondaryKey>
std::string disk_index<PrimaryKey, SecondaryKey>::index_name() const
{
    return _index_name;
}

template <class PrimaryKey, class SecondaryKey>
uint64_t disk_index<PrimaryKey, SecondaryKey>::unique_terms(doc_id d_id) const
{
    return _unique_terms.at(d_id);
}

template <class PrimaryKey, class SecondaryKey>
void disk_index<PrimaryKey, SecondaryKey>::create_index(
        const std::string & config_file)
{
    // save the config file so we can recreate the tokenizer
    std::ifstream source_config{config_file.c_str(), std::ios::binary};
    std::ofstream dest_config{_index_name + "/config.toml", std::ios::binary};
    dest_config << source_config.rdbuf();

    // load the documents from the corpus
    auto docs = corpus::corpus::load(config_file);

    // create postings file
    uint32_t num_chunks = tokenize_docs(docs);
    merge_chunks(num_chunks, _index_name + "/postings.index");
    compress(_index_name + "/postings.index");

    save_mapping(_doc_id_mapping, _index_name + "/docids.mapping");
    save_mapping(_doc_sizes, _index_name + "/docsizes.counts");
    save_mapping(_term_bit_locations, _index_name + "/lexicon.index");
    save_mapping(_labels, _index_name + "/docs.labels");
    save_mapping(_unique_terms, _index_name + "/docs.uniqueterms");
    save_mapping(_compression_mapping, _index_name + "/keys.compressedmapping");
    _tokenizer->save_term_id_mapping(_index_name + "/termids.mapping");
    set_label_ids();

    _postings = std::unique_ptr<io::mmap_file>{
        new io::mmap_file{_index_name + "/postings.index"}
    };
}

template <class PrimaryKey, class SecondaryKey>
void disk_index<PrimaryKey, SecondaryKey>::calc_compression_mapping(
        const std::string & filename)
{
    std::ifstream in{filename};
    postings_data<PrimaryKey, SecondaryKey> pdata{PrimaryKey{0}};
    std::unordered_map<uint64_t, uint64_t> freqs;

    while(in >> pdata)
    {
        for(auto & c: pdata.counts())
        {
            ++freqs[c.first];
            ++freqs[*reinterpret_cast<const uint64_t*>(&c.second)];
        }
    }

    using pair_t = std::pair<uint64_t, uint64_t>;
    std::vector<pair_t> sorted{freqs.begin(), freqs.end()};
    std::sort(sorted.begin(), sorted.end(),
        [](const pair_t & a, const pair_t & b) {
            return a.second > b.second;
        }
    );

    _compression_mapping.clear();

    // have to know what the delimiter is, and can't use 0
    uint64_t delim = std::numeric_limits<uint64_t>::max();
    _compression_mapping.insert(delim, 1);

    // 2 is the first valid compressed char after the delimiter 1
    uint64_t counter = 2;
    for(auto & p: sorted)
        _compression_mapping.insert(p.first, counter++);
}

template <class PrimaryKey, class SecondaryKey>
void disk_index<PrimaryKey, SecondaryKey>::compress(
        const std::string & filename)
{
    std::cerr << "Calculating optimal compression mapping..." << std::endl;

    calc_compression_mapping(filename);
    std::string cfilename{filename + ".compressed"};

    std::cerr << "Creating compressed postings file..." << std::endl;

    // create scope so the writer closes and we can calculate the size of the
    // file as well as rename it
    {
        io::compressed_file_writer out{cfilename, _compression_mapping};

        postings_data<PrimaryKey, SecondaryKey> pdata{PrimaryKey{0}};
        std::ifstream in{filename};
        while(in >> pdata)
        {
            _term_bit_locations[pdata.primary_key()] = out.bit_location();
            pdata.write_compressed(out);
        }
    }

    struct stat st;
    stat(cfilename.c_str(), &st);
    std::cerr << "Created compressed postings file ("
         << common::bytes_to_units(st.st_size) << ")" << std::endl;

    remove(filename.c_str());
    rename(cfilename.c_str(), filename.c_str());
}

template <class PrimaryKey, class SecondaryKey>
void disk_index<PrimaryKey, SecondaryKey>::load_index()
{
    std::cerr << "Loading index from disk ("
         << _index_name << ")..." << std::endl;

    auto config = cpptoml::parse_file(_index_name + "/config.toml");

    load_mapping(_doc_id_mapping, _index_name + "/docids.mapping");
    load_mapping(_doc_sizes, _index_name + "/docsizes.counts");
    load_mapping(_term_bit_locations, _index_name + "/lexicon.index");
    load_mapping(_labels, _index_name + "/docs.labels");
    load_mapping(_unique_terms, _index_name + "/docs.uniqueterms");
    load_mapping(_compression_mapping, _index_name + "/keys.compressedmapping");
    _tokenizer = tokenizers::tokenizer::load_tokenizer(config);
    _tokenizer->set_term_id_mapping(_index_name + "/termids.mapping");
    set_label_ids();

    _postings = std::unique_ptr<io::mmap_file>{
        new io::mmap_file{_index_name + "/postings.index"}
    };
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
void disk_index<PrimaryKey, SecondaryKey>::write_chunk(
        uint32_t chunk_num,
        std::vector<postings_data<PrimaryKey, SecondaryKey>> & pdata)
{
    std::sort(pdata.begin(), pdata.end());

    std::ofstream outfile{"chunk-" + common::to_string(chunk_num)};
    for(auto & p: pdata)
        outfile << p;
    outfile.close();

    pdata.clear();
}

template <class PrimaryKey, class SecondaryKey>
void disk_index<PrimaryKey, SecondaryKey>::merge_chunks(
        uint32_t num_chunks,
        const std::string & filename)
{
    using chunk_t = chunk<PrimaryKey, SecondaryKey>;

    // create priority queue of all chunks based on size
    std::priority_queue<chunk_t> chunks;
    for(uint32_t i = 0; i < num_chunks; ++i)
    {
        std::string filename = "chunk-" + common::to_string(i);
        chunks.push(chunk_t{filename});
    }

    // merge the smallest two chunks together until there is only one left
    while(chunks.size() > 1)
    {
        chunk_t first = chunks.top();
        chunks.pop();
        chunk_t second = chunks.top();
        chunks.pop();

        std::cerr << " Merging " << first.path() << " ("
             << common::bytes_to_units(first.size())
             << ") and " << second.path() << " ("
             << common::bytes_to_units(second.size())
             << "), " << chunks.size() << " remaining        \r";

        first.merge_with(second);
        chunks.push(first);
    }
    std::cerr << std::endl;

    rename(chunks.top().path().c_str(), filename.c_str());

    std::cerr << "Created uncompressed postings file " << filename
              << " (" << common::bytes_to_units(chunks.top().size()) << ")"
              << std::endl;
}

template <class PrimaryKey, class SecondaryKey>
template <class... Targs, template <class...> class Map>
void disk_index<PrimaryKey, SecondaryKey>::save_mapping(
        const Map<Targs...> & map,
        const std::string & filename) const
{
    std::ofstream outfile{filename};
    for(auto & p: map)
        outfile << p.first << " " << p.second << "\n";
    outfile.close();
}

template <class PrimaryKey, class SecondaryKey>
template <class Key, class Value, class... Targs, template <class...> class Map>
void disk_index<PrimaryKey, SecondaryKey>::load_mapping(
        Map<Key, Value, Targs...> & map, const std::string & filename)
{
    std::ifstream input{filename};
    Key k;
    Value v;
    while((input >> k) && (input >> v))
        map.insert(std::make_pair(k, v));
}

template <class PrimaryKey, class SecondaryKey>
double disk_index<PrimaryKey, SecondaryKey>::doc_size(doc_id d_id) const
{
    return _doc_sizes.at(d_id);
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
void disk_index<PrimaryKey, SecondaryKey>::tokenize(corpus::document & doc)
{
    _tokenizer->tokenize(doc);
}

template <class PrimaryKey, class SecondaryKey>
std::shared_ptr<postings_data<PrimaryKey, SecondaryKey>>
disk_index<PrimaryKey, SecondaryKey>::search_primary(PrimaryKey p_id) const
{
    using PostingsData = postings_data<PrimaryKey, SecondaryKey>;
    auto it = _term_bit_locations.find(p_id);

    // if the term doesn't exist in the index, return an empty postings_data
    if(it == _term_bit_locations.end())
        return std::make_shared<PostingsData>(p_id);

    io::compressed_file_reader reader{*_postings, _compression_mapping};
    reader.seek(it->second);

    auto pdata = std::make_shared<PostingsData>(PrimaryKey{p_id});
    pdata->read_compressed(reader);

    return pdata;
}

}
}
