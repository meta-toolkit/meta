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

class forward_index;

/**
 * A specialization of the traits class for forward indexes.
 */
template <>
struct index_traits<forward_index> {
    using primary_key_type   = doc_id;
    using secondary_key_type = term_id;
    using postings_data_type = postings_data<doc_id, term_id>;
};

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
class forward_index: public disk_index<forward_index>
{
    using base = disk_index<forward_index>;
    friend base;

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
         * @param d_id The document id of the doc to convert to liblinear format
         * @return the string representation liblinear format
         */
        std::string liblinear_data(doc_id d_id) const;

    private:
        /**
         * The chunk handler for forward indexes.
         */
        class chunk_handler : public base::chunk_handler<chunk_handler> {
            /** the current in-memory chunk */
            std::vector<postings_data<doc_id, term_id>> pdata_;

            /** the current size of the in-memory chunk */
            uint64_t chunk_size_{0};

            public:
                // inherit the base class constructor
                using base::chunk_handler<chunk_handler>::chunk_handler;

                /**
                 * Handler for when a given doc has been successfully
                 * tokenized.
                 */
                void handle_doc(const corpus::document & doc);

                /**
                 * Returns an in-memory chunk ready for being written to
                 * the disk.
                 */
                std::vector<postings_data<doc_id, term_id>> chunk();

                /**
                 * Destroys the handler, writing to disk any chunk data
                 * still resident in memory.
                 */
                ~chunk_handler();
        };

        /**
         * forward_index is a friend of the factory method used to create
         * it.
         */
        friend forward_index make_index<forward_index>(const std::string &);
};

}
}

#endif
