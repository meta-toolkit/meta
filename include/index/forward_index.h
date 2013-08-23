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
#include "meta.h"

namespace meta {
namespace index {

/**
 * stuff
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
class forward_index: public disk_index
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
         * @param t_id The term_id to search for
         * @param d_id The doc_id to search for
         * @note This function is not const because the cache may be updated
         */
        uint64_t term_freq(term_id t_id, doc_id d_id);

        /**
         * @param d_id The doc id to find containing terms from
         * @return a mapping of term_id -> term occurrence
         */
        const std::unordered_map<term_id, uint64_t> counts(doc_id d_id);

        /**
         * @param d_id The doc id to find the class label for
         * @return the label of the class that the document belongs to, or an
         * empty string if a label was not assigned
         */
        class_label label(doc_id d_id) const;

    protected:
        /**
         * @param docs The documents to add to the inverted index
         */
        uint32_t tokenize_docs(std::vector<document> & docs);

    private:
        /** maps which class a document belongs to (if any) */
        std::unordered_map<doc_id, class_label> _labels;
};

}
}

#endif
