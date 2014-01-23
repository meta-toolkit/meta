/**
 * @file forward_index.cpp
 * @author Sean Massung
 */

#include <iostream>
#include "index/forward_index.h"

namespace meta {
namespace index {

forward_index::forward_index(const cpptoml::toml_group & config):
    _index_name{*cpptoml::get_as<std::string>(config, "forward-index")}
{ /* nothing */ }

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
        create_univerted_metadata(config);
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
}

void forward_index::uninvert(const cpptoml::toml_group& config)
{
}

void forward_index::create_univerted_metadata(const cpptoml::toml_group& config)
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

class_label forward_index::label(doc_id d_id) const
{
    return class_label{""};
}

class_label forward_index::class_label_from_id(label_id l_id) const
{
    return class_label{""};
}

uint64_t forward_index::num_docs() const
{
    return 0;
}

std::vector<doc_id> forward_index::docs() const
{
    std::vector<doc_id> vec;
    return vec;
}

uint64_t forward_index::unique_terms() const
{
    return 0;
}

auto forward_index::search_primary(doc_id d_id) const
    -> std::shared_ptr<postings_data_type>
{
    auto pdata = std::make_shared<postings_data_type>(doc_id{0});
    return pdata;
}

}
}
