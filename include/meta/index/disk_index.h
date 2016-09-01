/**
 * @file disk_index.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_DISK_INDEX_H_
#define META_DISK_INDEX_H_

#include <memory>
#include <vector>

#include "meta/config.h"
#include "meta/corpus/metadata.h"
#include "meta/meta.h"
#include "meta/util/pimpl.h"

namespace cpptoml
{
class table;
}

namespace meta
{

namespace index
{
class string_list;
class vocabulary_map;
}

namespace util
{
template <class>
class disk_vector;
}
}

namespace meta
{
namespace index
{

/**
 * Holds generic data structures and functions that inverted_index and
 * forward_index both use. Provides common interface for both and is implemented
 * using the pointer-to-implementation method.
 */
class disk_index
{
  public:
    /**
     * Default destructor.
     */
    virtual ~disk_index() = default;

    /**
     * @return the name of this index
     */
    std::string index_name() const;

    /**
     * @return the number of documents in this index
     */
    uint64_t num_docs() const;

    /**
     * @param d_id
     * @return the actual name of this document
     */
    std::string doc_name(doc_id d_id) const;

    /**
     * @param d_id
     * @return the path to the file containing this document
     */
    std::string doc_path(doc_id d_id) const;

    /**
     * @return a vector of doc_ids that are contained in this index
     */
    std::vector<doc_id> docs() const;

    /**
     * @param d_id The document to search for
     * @return the size of the given document (the total number of terms
     * occurring)
     */
    uint64_t doc_size(doc_id d_id) const;

    /**
     * @param d_id The doc id to find the class label for
     * @return the label of the class that the document belongs to, or an
     * empty string if a label was not assigned
     */
    class_label label(doc_id d_id) const;

    /**
     * @param d_id The doc id to find the label_id for
     * @return the label_id of the class that to document belongs to
     */
    label_id lbl_id(doc_id d_id) const;

    /**
     * @param label The class label
     * @return the label_id for the given class label
     */
    label_id id(class_label label) const;

    /**
     * @param l_id The id of the class label in question
     * @return the integer label id of a document
     */
    class_label class_label_from_id(label_id l_id) const;

    /**
     * @return the number of labels in this index
     */
    uint64_t num_labels() const;

    /**
     * @return the distinct class labels possible for documents in this
     * index
     */
    std::vector<class_label> class_labels() const;

    /**
     * @param d_id The document id to fetch metadata for
     * @return the metadata associated with this document id
     */
    corpus::metadata metadata(doc_id d_id) const;

    /**
     * @param d_id
     * @return the number of unique terms in d_id
     */
    virtual uint64_t unique_terms(doc_id d_id) const;

    /**
     * @return the number of unique terms in the index
     */
    virtual uint64_t unique_terms() const;

    /**
     * @param term
     * @return the term_id associated with the parameter
     */
    term_id get_term_id(const std::string& term);

    /**
     * @param t_id The term_id to get the original text for
     * @return the string representation of the term
     */
    std::string term_text(term_id t_id) const;

  protected:
    /// Forward declare the implementation
    class disk_index_impl;
    /// Implementation of this disk_index
    util::pimpl<disk_index_impl> impl_;

    /**
     * Constructor.
     * @param config The config settings used to create this index
     * @param name The name of this disk_index
     */
    disk_index(const cpptoml::table& config, const std::string& name);

    /**
     * disk_index may not be copy-constructed.
     */
    disk_index(const disk_index&) = delete;

    /**
     * disk_index may not be copy-assigned.
     */
    disk_index& operator=(const disk_index&) = delete;

  public:
    /**
     * Move constructs a disk_index.
     **/
    disk_index(disk_index&&) = default;

    /**
     * Move assigns a disk_index.
     */
    disk_index& operator=(disk_index&&) = default;
};
}
}

#endif
