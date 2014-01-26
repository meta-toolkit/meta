/**
 * @file forward_index.cpp
 * @author Sean Massung
 */

#include "index/forward_index.h"
#include "index/postings_data.h"
#include "index/string_list.h"
#include "index/string_list_writer.h"
#include "index/vocabulary_map.h"
#include "tokenizers/tokenizer.h"
#include "util/disk_vector.h"

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
}

void forward_index::create_index(const std::string & config_file)
{
    LOG(info) << "Creating index: " << _index_name << ENDLG;

    filesystem::copy_file(config_file, _index_name + "/config.toml");
    auto config = cpptoml::parse_file(_index_name + "/config.toml");

    // if the corpus is a single libsvm formatted file, then we are done;
    // otherwise, we will create an inverted index and the uninvert it
    if(is_libsvm_format(config))
    {
        create_libsvm_postings(config);
        create_libsvm_metadata(config);
    }
    else
    {
        uninvert(config);
        create_uninverted_metadata(config);
    }

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
}

void forward_index::create_libsvm_metadata(const cpptoml::toml_group& config)
{
    doc_id d_id{0};
    std::unordered_set<term_id> terms;

    std::ifstream in{_index_name + "/postings.index"};
    std::string line;
    while(in.good())
    {
        std::getline(in, line);
        if(line.empty())
            break;

        std::stringstream stream{line};
        std::string token;
        stream >> token;
        (*_labels)[d_id] = get_label_id(class_label{token});

        uint64_t num_unique = 0;
        uint64_t length = 0;
        double count = 0.0;
        term_id term;
        while(stream >> token)
        {
            ++num_unique;
            size_t idx = token.find_first_of(':');
            std::string feature = token.substr(0, idx);
            std::istringstream{feature} >> term;
            std::istringstream{token.substr(idx + 1)} >> count;
            length += static_cast<uint64_t>(count); // TODO
        }

        //_doc_id_mapping->insert(d_id, "[no path]");
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
    auto pdata = std::make_shared<postings_data_type>(doc_id{0});
    return pdata;
}

}
}
