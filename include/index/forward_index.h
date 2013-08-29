/**
 * @file forward_index.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _FORWARD_INDEX_H_
#define _FORWARD_INDEX_H_

#include <string>
#include <vector>
#include <memory>
#include "index/disk_index.h"
#include "util/invertible_map.h"
#include "meta.h"

namespace meta {
namespace index {

/**
 * The forward_index stores information on a corpus by doc_ids.  Each doc_id key
 * is associated with a distribution of term_ids or term "counts" that occur in
 * that particular document.
 *
 * A forward index consists of the following five files:
 *  - termids.mapping: maps term_id -> string information
 *  - docids.mapping: maps doc_id -> document path
 *  - postings.index: the large file saved on disk for per-document term_id
 *      information
 *  - lexicon.index: the smaller file that contains pointers into the postings
 *      file based on term_id
 *  - docsizes.counts: maps doc_id -> number of terms
 *  - config.toml: saves the tokenizer configuration
 */
class forward_index: public disk_index<doc_id, term_id>
{
    protected:
        /**
         * @param config The toml_group that specifies how to create the
         * index.
         */
        forward_index(const cpptoml::toml_group & config);

    public:
        /**
         * Move constructs a forward_index.
         * @param other The forward_index to move into this one.
         */
        forward_index(forward_index && other) = default;

        /**
         * Move assigns a forward_index.
         * @param other The forward_index to move into this one.
         */
        forward_index & operator=(forward_index && other) = default;

        /**
         * forward_index may not be copy-constructed.
         */
        forward_index(const forward_index &) = delete;

        /**
         * forward_index may not be copy-assigned.
         */
        forward_index & operator=(const forward_index &) = delete;

        /**
         * Default destructor.
         */
        virtual ~forward_index() = default;

        /**
         * @param d_id The doc id to find containing terms from
         * @return a mapping of term_id -> term occurrence
         */
        const std::unordered_map<term_id, uint64_t> counts(doc_id d_id) const;

        /**
         * @param d_id The doc id to find the class label for
         * @return the label of the class that the document belongs to, or an
         * empty string if a label was not assigned
         */
        class_label label(doc_id d_id) const;

        /**
         * @param d_id The document id of the doc to convert to liblinear format
         * @return the string representation liblinear format
         */
        std::string liblinear_data(doc_id d_id) const;

        /**
         * @param l_id The id of the class label in question
         * @return the integer label id of a document
         */
        class_label class_label_from_id(label_id l_id) const;

    protected:
        /**
         * @param docs The documents to add to the inverted index
         */
        uint32_t tokenize_docs(std::vector<document> & docs);

    private:
        /**
         * Initializes the _label_ids member.
         */
        void set_label_ids();

        /**
         * forward_index is a friend of the factory method used to create
         * it.
         */
        friend forward_index make_index<forward_index>(const std::string &);

        /** maps which class a document belongs to (if any) */
        std::unordered_map<doc_id, class_label> _labels;

        /** assigns an integer to each class label (used for liblinear and slda
         * mappings) */
        util::invertible_map<class_label, label_id> _label_ids;

};

}
}

#endif
