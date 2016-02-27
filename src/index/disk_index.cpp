/**
 * @file disk_index.cpp
 * @author Sean Massung
 */

#include <numeric>
#include <stdexcept>

#include "meta/index/disk_index.h"
#include "meta/index/disk_index_impl.h"
#include "meta/index/string_list.h"
#include "meta/index/string_list_writer.h"
#include "meta/index/vocabulary_map.h"
#include "meta/analyzers/analyzer.h"
#include "meta/util/disk_vector.h"
#include "meta/util/mapping.h"
#include "meta/util/optional.h"
#include "meta/util/pimpl.tcc"

namespace meta
{
namespace index
{

disk_index::disk_index(const cpptoml::table&, const std::string& name)
{
    impl_->index_name_ = name;
}

std::string disk_index::index_name() const
{
    return impl_->index_name_;
}

term_id disk_index::get_term_id(const std::string& term)
{
    std::lock_guard<std::mutex> lock{impl_->mutex_};

    auto termID = impl_->term_id_mapping_->find(term);
    if (termID)
        return term_id{*termID};

    uint64_t size = impl_->term_id_mapping_->size();
    return term_id{size};
}

class_label disk_index::label(doc_id d_id) const
{
    return class_label_from_id(impl_->labels_->at(d_id));
}

label_id disk_index::lbl_id(doc_id d_id) const
{
    return impl_->labels_->at(d_id);
}

label_id disk_index::id(class_label label) const
{
    if (!impl_->label_ids_.contains_key(label))
        throw std::out_of_range{"Invalid class_label: " + std::string(label)};
    return impl_->label_ids_.get_value(label);
}

class_label disk_index::class_label_from_id(label_id l_id) const
{
    if (!impl_->label_ids_.contains_value(l_id))
        throw std::out_of_range{"Invalid label_id: " + std::to_string(l_id)};
    return impl_->label_ids_.get_key(l_id);
}

uint64_t disk_index::num_labels() const
{
    return impl_->label_ids_.size();
}

std::vector<class_label> disk_index::class_labels() const
{
    return impl_->class_labels();
}

corpus::metadata disk_index::metadata(doc_id d_id) const
{
    return impl_->metadata_->get(d_id);
}

uint64_t disk_index::unique_terms(doc_id d_id) const
{
    return *metadata(d_id).get<uint64_t>("unique-terms");
}

uint64_t disk_index::unique_terms() const
{
    return impl_->term_id_mapping_->size();
}

uint64_t disk_index::doc_size(doc_id d_id) const
{
    return *metadata(d_id).get<uint64_t>("length");
}

uint64_t disk_index::num_docs() const
{
    return impl_->metadata_->size();
}

std::string disk_index::doc_name(doc_id d_id) const
{
    auto path = doc_path(d_id);
    return path.substr(path.find_last_of("/") + 1);
}

std::string disk_index::doc_path(doc_id d_id) const
{
    if (auto path = impl_->metadata_->get(d_id).get<std::string>("path"))
        return *path;
    return "[none]";
}

std::vector<doc_id> disk_index::docs() const
{
    std::vector<doc_id> ret(num_docs());
    std::iota(ret.begin(), ret.end(), 0_did);
    return ret;
}

// disk_index_impl

const std::vector<const char*> disk_index::disk_index_impl::files
    = {"/docs.labels",          "/labelids.mapping", "/postings.index",
       "/postings.index_index", "/termids.mapping",  "/termids.mapping.inverse",
       "/metadata.db",          "/metadata.index"};

label_id disk_index::disk_index_impl::get_label_id(const class_label& lbl)
{
    std::lock_guard<std::mutex> lock{mutex_};
    if (!label_ids_.contains_key(lbl))
    {
        // SVM multiclass has label_ids starting at 1
        label_id next_id{static_cast<uint32_t>(label_ids_.size() + 1)};
        label_ids_.insert(lbl, next_id);
        return next_id;
    }
    else
        return label_ids_.get_value(lbl);
}

void disk_index::disk_index_impl::initialize_metadata()
{
    metadata_ = {index_name_};
}

void disk_index::disk_index_impl::load_labels(uint64_t num_docs)
{
    // clear the current label set; this is so that the disk vector can
    // flush via munmap() if needed
    labels_ = util::nullopt;

    // load in the new mapping
    labels_ = util::disk_vector<label_id>{index_name_ + files[DOC_LABELS],
                                          num_docs};
}

void disk_index::disk_index_impl::load_term_id_mapping()
{
    term_id_mapping_ = vocabulary_map{index_name_ + files[TERM_IDS_MAPPING]};
}

void disk_index::disk_index_impl::load_label_id_mapping()
{
    map::load_mapping(label_ids_, index_name_ + files[LABEL_IDS_MAPPING]);
}

void disk_index::disk_index_impl::save_label_id_mapping()
{
    map::save_mapping(label_ids_, index_name_ + files[LABEL_IDS_MAPPING]);
}

void disk_index::disk_index_impl::set_label(doc_id id, const class_label& label)
{
    (*labels_)[id] = get_label_id(label);
}

uint64_t disk_index::disk_index_impl::total_unique_terms() const
{
    return term_id_mapping_->size();
}

label_id disk_index::disk_index_impl::doc_label_id(doc_id id) const
{
    return labels_->at(id);
}

std::vector<class_label> disk_index::disk_index_impl::class_labels() const
{
    std::vector<class_label> labels;
    labels.reserve(label_ids_.size());
    for (const auto& pair : label_ids_)
        labels.emplace_back(pair.first);
    return labels;
}

std::string disk_index::term_text(term_id t_id) const
{
    if (t_id >= impl_->term_id_mapping_->size())
        return "";
    return impl_->term_id_mapping_->find_term(t_id);
}
}
}
