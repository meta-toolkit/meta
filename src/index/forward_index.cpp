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

}

void forward_index::create_index(const std::string & config_file)
{

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
