/**
 * @file disk_index.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _DISK_INDEX_H_
#define _DISK_INDEX_H_

#include <memory>
#include <mutex>

#include "caching/dblru_cache.h"
#include "util/invertible_map.h"
#include "meta.h"

namespace cpptoml {
class toml_group;
}

namespace meta {
namespace util {

template <class, class, template <class, class> class>
class sqlite_map;

template <class>
class disk_vector;
}

namespace tokenizers {
class tokenizer;
}

}


namespace meta {
namespace index {

class vocabulary_map;

/**
 * Holds generic data structures and functions that inverted_index and
 * forward_index both use.
 */
class disk_index
{
   protected:
    /**
     * Constructor.
     * @param config_file
     */
    disk_index(const cpptoml::toml_group & config);

    /**
     * disk_index may not be copy-constructed.
     */
    disk_index(const disk_index &) = delete;

    /**
     * disk_index may not be copy-assigned.
     */
    disk_index &operator=(const disk_index &) = delete;

    /**
     * Move constructs a disk_index.
     **/
    disk_index(disk_index&&);

    /**
     * Move assigns a disk_index.
     */
    disk_index& operator=(disk_index&&);

   public:
    /**
     * Default destructor.
     */
    virtual ~disk_index();

    /**
     * @return the number of documents in this index
     */
    uint64_t num_docs() const;

    /**
     * @param doc_id
     * @return the actual name of this document
     */
    std::string doc_name(doc_id d_id) const;

    /**
     * @param doc_id
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
     * @param l_id The id of the class label in question
     * @return the integer label id of a document
     */
    class_label class_label_from_id(label_id l_id) const;

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
    term_id get_term_id(const std::string & term);

   protected:
    /**
     * @param d_id The document
     * @return the numerical label_id for a given document's label
     */
    label_id label_id_from_doc(doc_id d_id) const;

    /**
     * doc_id -> document path mapping.
     * Each index corresponds to a doc_id (uint64_t).
     */
    std::unique_ptr<util::sqlite_map<doc_id, std::string,
                                     caching::default_dblru_cache>>
    _doc_id_mapping;

    /**
     * doc_id -> document length mapping.
     * Each index corresponds to a doc_id (uint64_t).
     */
    std::unique_ptr<util::disk_vector<double>> _doc_sizes;

    /** the tokenizer used to tokenize documents in the index */
    std::unique_ptr<tokenizers::tokenizer> _tokenizer;

    /**
     * Maps which class a document belongs to (if any).
     * Each index corresponds to a doc_id (uint64_t).
     */
    std::unique_ptr<util::disk_vector<label_id>> _labels;

    /**
     * Holds how many unique terms there are per-document. This is sort of
     * like an inverse IDF. For a forward_index, this field is certainly
     * redundant, though it can save querying the postings file.
     * Each index corresponds to a doc_id (uint64_t).
     */
    std::unique_ptr<util::disk_vector<uint64_t>> _unique_terms;

    /**
     * Maps string terms to term_ids.
     */
    std::unique_ptr<vocabulary_map> _term_id_mapping;

    /**
     * @param lbl the string class label to find the id for
     * @return the label_id of a class_label, creating a new one if
     * necessary
     */
    label_id get_label_id(const class_label & lbl);

    /**
     * assigns an integer to each class label (used for liblinear and slda
     * mappings)
     */
    util::invertible_map<class_label, label_id> _label_ids;

    /**
     * A pointer to a memory-mapped postings file. It is a pointer because
     * we want to delay the initialization of it until the postings file is
     * created in some cases.
     */
    std::unique_ptr<io::mmap_file> _postings;

    /** mutex for thread-safe operations */
    std::unique_ptr<std::mutex> _mutex{new std::mutex};

};

}
}

#endif
