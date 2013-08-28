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
    public:
        /**
         * @param index_name The name for this inverted index to be saved as
         * @param docs The untokenized documents to add to the index
         * @param tok The tokenizer to use to tokenize the documents
         * @param config_file The configuration file used to create the
         * tokenizer
         */
        forward_index(const std::string & index_name,
                      std::vector<document> & docs,
                      std::shared_ptr<tokenizers::tokenizer> & tok,
                      const std::string & config_file);

        /**
         * @param index_path The directory containing an already-created index
         */
        forward_index(const std::string & index_path);

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
         * @return a forward_index object created from the config file; if an index
         * at the given path already exists, it will load that one
         */
        static std::unique_ptr<forward_index> load_index(const cpptoml::toml_group & config);

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

        /** maps which class a document belongs to (if any) */
        std::unordered_map<doc_id, class_label> _labels;

        /** assigns an integer to each class label (used for liblinear and slda
         * mappings) */
        util::invertible_map<class_label, label_id> _label_ids;

};

}
}

#endif
