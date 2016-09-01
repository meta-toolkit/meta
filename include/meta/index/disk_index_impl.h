/**
 * @file disk_index_impl.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INDEX_DISK_INDEX_IMPL_H_
#define META_INDEX_DISK_INDEX_IMPL_H_

#include <mutex>

#include "meta/config.h"
#include "meta/index/disk_index.h"
#include "meta/index/metadata_file.h"
#include "meta/index/string_list.h"
#include "meta/index/vocabulary_map.h"
#include "meta/util/disk_vector.h"
#include "meta/util/invertible_map.h"
#include "meta/util/optional.h"

namespace meta
{
namespace index
{

class string_list_writer;

/**
 * Collection of all the files that comprise a disk_index.
 */
enum index_file
{
    DOC_LABELS,
    LABEL_IDS_MAPPING,
    POSTINGS,
    POSTINGS_INDEX,
    TERM_IDS_MAPPING,
    TERM_IDS_MAPPING_INVERSE,
    METADATA_DB,
    METADATA_INDEX
};

/**
 * The implementation of a disk_index.
 */
class disk_index::disk_index_impl
{
  public:
    /// friend the interface
    friend disk_index;

    /**
     * Filenames used in the index.
     */
    const static std::vector<const char*> files;

    /**
     * Loads the metadata file.
     */
    void initialize_metadata();

    /**
     * Loads the doc labels.
     * @param num_docs The number of documents stored in the index
     */
    void load_labels(uint64_t num_docs = 0);

    /**
     * Loads the term_id mapping.
     */
    void load_term_id_mapping();

    /**
     * Loads the label_id mapping.
     */
    void load_label_id_mapping();

    /**
     * Saves the label_id mapping.
     */
    void save_label_id_mapping();

    /**
     * Sets the label for a document.
     * @param id The document id
     * @param label The new label
     */
    void set_label(doc_id id, const class_label& label);

    /**
     * @return the total number of unique terms in the index.
     */
    uint64_t total_unique_terms() const;

    /**
     * @return the label id for a given document.
     * @param id The document id
     */
    label_id doc_label_id(doc_id id) const;

    /**
     * @return the possible class labels for this index
     */
    std::vector<class_label> class_labels() const;

  private:
    /**
     * @param lbl the string class label to find the id for
     * @return the label_id of a class_label, creating a new one if
     * necessary
     */
    label_id get_label_id(const class_label& lbl);

    /// the location of this index
    std::string index_name_;

    /**
     * Maps which class a document belongs to (if any).
     * Each index corresponds to a doc_id (uint64_t).
     */
    util::optional<util::disk_vector<label_id>> labels_;

    /// Stores additional metadata for each document
    util::optional<metadata_file> metadata_;

    /// Maps string terms to term_ids.
    util::optional<vocabulary_map> term_id_mapping_;

    /// Assigns an integer to each class label (used for liblinear mappings)
    util::invertible_map<class_label, label_id> label_ids_;

    /// mutex for thread-safe operations
    mutable std::mutex mutex_;
};
}
}
#endif
