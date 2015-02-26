/**
 * @file disk_index.cpp
 * @author Sean Massung
 */

#include <numeric>

#include "index/disk_index.h"
#include "index/disk_index_impl.h"
#include "index/string_list.h"
#include "index/string_list_writer.h"
#include "index/vocabulary_map.h"
#include "analyzers/analyzer.h"
#include "util/disk_vector.h"
#include "util/mapping.h"
#include "util/optional.h"
#include "util/pimpl.tcc"

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
    return impl_->label_ids_.get_value(label);
}

class_label disk_index::class_label_from_id(label_id l_id) const
{
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

uint64_t disk_index::unique_terms(doc_id d_id) const
{
    return impl_->unique_terms_->at(d_id);
}

uint64_t disk_index::unique_terms() const
{
    return impl_->term_id_mapping_->size();
}

uint64_t disk_index::doc_size(doc_id d_id) const
{
    return impl_->doc_sizes_->at(d_id);
}

uint64_t disk_index::num_docs() const
{
    return impl_->doc_sizes_->size();
}

std::string disk_index::doc_name(doc_id d_id) const
{
    auto path = doc_path(d_id);
    return path.substr(path.find_last_of("/") + 1);
}

std::string disk_index::doc_path(doc_id d_id) const
{
    return impl_->doc_id_mapping_->at(d_id);
}

std::vector<doc_id> disk_index::docs() const
{
    std::vector<doc_id> ret(impl_->doc_id_mapping_->size());
    std::iota(ret.begin(), ret.end(), 0);
    return ret;
}

// disk_index_impl

const std::vector<const char*> disk_index::disk_index_impl::files
    = {"/docids.mapping", "/docids.mapping_index", "/docsizes.counts",
       "/docs.labels",    "/docs.uniqueterms",     "/labelids.mapping",
       "/postings.index", "/termids.mapping",      "/termids.mapping.inverse"};

label_id disk_index::disk_index_impl::get_label_id(const class_label& lbl)
{
    std::lock_guard<std::mutex> lock{mutex_};
    if (!label_ids_.contains_key(lbl))
    {
        // SVM multiclass has label_ids starting at 1
        label_id next_id{static_cast<label_id>(label_ids_.size() + 1)};
        label_ids_.insert(lbl, next_id);
        return next_id;
    }
    else
        return label_ids_.get_value(lbl);
}

void disk_index::disk_index_impl::initialize_metadata(uint64_t num_docs)
{
    load_doc_sizes(num_docs);
    load_labels(num_docs);
    load_unique_terms(num_docs);
}

void disk_index::disk_index_impl::load_doc_sizes(uint64_t num_docs)
{
    doc_sizes_
        = util::disk_vector<double>{index_name_ + files[DOC_SIZES], num_docs};
}

void disk_index::disk_index_impl::load_labels(uint64_t num_docs)
{
    labels_ = util::disk_vector<label_id>{index_name_ + files[DOC_LABELS],
                                          num_docs};
}

void disk_index::disk_index_impl::load_unique_terms(uint64_t num_docs)
{
    unique_terms_ = util::disk_vector<uint64_t>{
        index_name_ + files[DOC_UNIQUETERMS], num_docs};
}

void disk_index::disk_index_impl::load_doc_id_mapping()
{
    doc_id_mapping_ = string_list{index_name_ + files[DOC_IDS_MAPPING]};
}

void disk_index::disk_index_impl::load_term_id_mapping()
{
    term_id_mapping_ = vocabulary_map{index_name_ + files[TERM_IDS_MAPPING]};
}

void disk_index::disk_index_impl::load_label_id_mapping()
{
    map::load_mapping(label_ids_, index_name_ + files[LABEL_IDS_MAPPING]);
}

void disk_index::disk_index_impl::load_postings()
{
    postings_ = io::mmap_file{index_name_ + files[POSTINGS]};
}

void disk_index::disk_index_impl::save_label_id_mapping()
{
    map::save_mapping(label_ids_, index_name_ + files[LABEL_IDS_MAPPING]);
}

string_list_writer
    disk_index::disk_index_impl::make_doc_id_writer(uint64_t num_docs) const
{
    return {index_name_ + files[DOC_IDS_MAPPING], num_docs};
}

void disk_index::disk_index_impl::set_label(doc_id id, const class_label& label)
{
    (*labels_)[id] = get_label_id(label);
}

void disk_index::disk_index_impl::set_length(doc_id id, uint64_t length)
{
    (*doc_sizes_)[id] = length;
}

void disk_index::disk_index_impl::set_unique_terms(doc_id id, uint64_t terms)
{
    (*unique_terms_)[id] = terms;
}

const io::mmap_file& disk_index::disk_index_impl::postings() const
{
    return *postings_;
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
