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

#include "index/disk_index.h"
#include "index/string_list.h"
#include "index/vocabulary_map.h"
#include "util/disk_vector.h"
#include "util/invertible_map.h"
#include "util/optional.h"

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
    DOC_IDS_MAPPING = 0,
    DOC_IDS_MAPPING_INDEX,
    DOC_SIZES,
    DOC_LABELS,
    DOC_UNIQUETERMS,
    LABEL_IDS_MAPPING,
    POSTINGS,
    TERM_IDS_MAPPING,
    TERM_IDS_MAPPING_INVERSE
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
     * Initializes the following metadata maps:
     * doc_sizes_, labels_, unique_terms_
     * @param num_docs The number of documents stored in the index
     */
    void initialize_metadata(uint64_t num_docs = 0);

    /**
     * Loads the doc sizes.
     * @param num_docs The number of documents stored in the index
     */
    void load_doc_sizes(uint64_t num_docs = 0);

    /**
     * Loads the doc labels.
     * @param num_docs The number of documents stored in the index
     */
    void load_labels(uint64_t num_docs = 0);

    /**
     * Loads the unique terms per document.
     * @param num_docs The number of documents stored in the index
     */
    void load_unique_terms(uint64_t num_docs = 0);

    /**
     * Loads the doc_id mapping.
     */
    void load_doc_id_mapping();

    /**
     * Loads the term_id mapping.
     */
    void load_term_id_mapping();

    /**
     * Loads the label_id mapping.
     */
    void load_label_id_mapping();

    /**
     * Loads the postings file.
     */
    void load_postings();

    /**
     * Saves the label_id mapping.
     */
    void save_label_id_mapping();

    /**
     * Creates a string_list_writer for writing the docids mapping.
     * @param num_docs The number of documents stored in the index, as the size
     * of the string_list_writer
     * @return the string_list_writer to write doc ids
     */
    string_list_writer make_doc_id_writer(uint64_t num_docs) const;

    /**
     * Sets the label for a document.
     * @param id The document id
     * @param label The new label
     */
    void set_label(doc_id id, const class_label& label);

    /**
     * Sets the size of a document.
     * @param id The document id
     * @param length The number of terms that will appear in the document
     */
    void set_length(doc_id id, uint64_t length);

    /**
     * Sets the number of unique terms for a document.
     * @param id The document id
     * @param terms The number of unique terms that will appear in the document
     */
    void set_unique_terms(doc_id id, uint64_t terms);

    /**
     * @return the mmap file for the postings.
     */
    const io::mmap_file& postings() const;

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
     * doc_id -> document path mapping.
     * Each index corresponds to a doc_id (uint64_t).
     */
    util::optional<string_list> doc_id_mapping_;

    /**
     * doc_id -> document length mapping.
     * Each index corresponds to a doc_id (uint64_t).
     */
    util::optional<util::disk_vector<double>> doc_sizes_;

    /**
     * Maps which class a document belongs to (if any).
     * Each index corresponds to a doc_id (uint64_t).
     */
    util::optional<util::disk_vector<label_id>> labels_;

    /**
     * Holds how many unique terms there are per-document. This is sort of
     * like an inverse IDF. For a forward_index, this field is certainly
     * redundant, though it can save querying the postings file.
     * Each index corresponds to a doc_id (uint64_t).
     */
    util::optional<util::disk_vector<uint64_t>> unique_terms_;

    /// Maps string terms to term_ids.
    util::optional<vocabulary_map> term_id_mapping_;

    /// Assigns an integer to each class label (used for liblinear mappings)
    util::invertible_map<class_label, label_id> label_ids_;

    /**
     * A pointer to a memory-mapped postings file. It is a pointer because
     * we want to delay the initialization of it until the postings file is
     * created in some cases.
     */
    util::optional<io::mmap_file> postings_;

    /// mutex for thread-safe operations
    mutable std::mutex mutex_;
};
}
}
#endif
