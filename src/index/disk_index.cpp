/**
 * @file disk_index.cpp
 * @author Sean Massung
 */

#include <numeric>

#include "index/disk_index.h"
#include "index/vocabulary_map.h"
#include "tokenizers/tokenizer.h"
#include "util/sqlite_map.h"
#include "util/disk_vector.h"

namespace meta {
namespace index {

disk_index::disk_index(const cpptoml::toml_group & config):
    _tokenizer{tokenizers::tokenizer::load(config)}
{ /* nothing */}

disk_index::disk_index(disk_index&&) = default;
disk_index::~disk_index() = default;
disk_index& disk_index::operator=(disk_index&&) = default;

term_id disk_index::get_term_id(const std::string & term)
{
    std::lock_guard<std::mutex> lock{*_mutex};

    auto termID = _term_id_mapping->find(term);
    if(termID)
        return term_id{*termID};

    uint64_t size = _term_id_mapping->size();
    return term_id{size};
}

class_label disk_index::label(doc_id d_id) const
{
    return class_label_from_id(_labels->at(d_id));
}

class_label disk_index::class_label_from_id(label_id l_id) const
{
    return _label_ids.get_key(l_id);
}

label_id disk_index::get_label_id(const class_label & lbl)
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

label_id disk_index::label_id_from_doc(doc_id d_id) const
{
    return _labels->at(d_id);
}

uint64_t disk_index::unique_terms(doc_id d_id) const
{
    return _unique_terms->at(d_id);
}

uint64_t disk_index::unique_terms() const
{
    return _term_id_mapping->size();
}

uint64_t disk_index::doc_size(doc_id d_id) const
{
    return _doc_sizes->at(d_id);
}

uint64_t disk_index::num_docs() const
{
    return _doc_sizes->size();
}

std::string disk_index::doc_name(doc_id d_id) const
{
    auto path = doc_path(d_id);
    return path.substr(path.find_last_of("/") + 1);
}

std::string disk_index::doc_path(doc_id d_id) const
{
    return *_doc_id_mapping->find(d_id);
}

std::vector<doc_id> disk_index::docs() const
{
    std::vector<doc_id> ret(_doc_id_mapping->size());
    std::iota(ret.begin(), ret.end(), 0);
    return ret;
}

}
}
