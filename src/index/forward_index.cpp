/**
 * @file forward_index.cpp
 * @author Sean Massung
 */

#include "index/forward_index.h"
#include "index/postings_data.h"
#include "index/string_list.h"
#include "index/string_list_writer.h"
#include "index/vocabulary_map.h"
#include "io/libsvm_parser.h"
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
    return "";
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
    LOG(info) << "Creating index: " << _index_name << ENDLG;

    filesystem::copy_file(config_file, _index_name + "/config.toml");
    auto config = cpptoml::parse_file(_index_name + "/config.toml");
    _tokenizer = tokenizers::tokenizer::load(config);

    // if the corpus is a single libsvm formatted file, then we are done;
    // otherwise, we will create an inverted index and the uninvert it
    if(is_libsvm_format(config))
    {
        create_libsvm_postings(config);
        create_libsvm_metadata(config);
        _term_id_mapping = nullptr; // we don't know what the terms are!
    }
    else
    {
        uninvert(config);
        create_uninverted_metadata(config);
    }

    // now that the files are tokenized, we can create the string_list
    _doc_id_mapping = make_unique<string_list>(_index_name
                                                    + "/docids.mapping");
    map::save_mapping(_label_ids, _index_name + "/labelids.mapping");

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

void forward_index::uninvert(const cpptoml::toml_group& config)
{
}

void forward_index::create_uninverted_metadata(const cpptoml::toml_group& config)
{
    init_metadata();
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
    if(d_id >= num_docs())
        throw forward_index_exception{"invalid doc_id in search_primary"};

    uint64_t begin = (*_doc_byte_locations)[d_id];
    uint64_t length = 0;
    while(_postings->start()[begin + length] != '\n')
        ++length; // TODO maybe save lengths as well?

    auto pdata = std::make_shared<postings_data_type>(d_id);
    std::string line{_postings->start() + begin, length};
    pdata->set_counts(io::libsvm_parser::counts(line));
    return pdata;
}

}
}
