/**
 * @file postings_data.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _POSTINGS_DATA_
#define _POSTINGS_DATA_

#include <unordered_map>
#include <string>
#include "meta.h"

namespace meta {
namespace index {

/**
 * A class to represent the per-term_id data in the inverted_index's postings
 * file.
 */
class postings_data
{
    public:
        /**
         * @param t_id The term_id to be associated with this postings_data
         * Creates an empty postings_data for a given term_id
         */
        postings_data(term_id t_id);

        /**
         * @param raw_data The raw data from the postings file (a list of
         * numbers)
         * This function converts a list of numbers into a postings_data object
         */
        postings_data(const std::string & raw_data);

        /**
         * @return the string representation of this postings_data (a sequence
         * of bytes)
         */
        std::string to_string() const;

        /**
         * @param d_id The doc_id of the new document to add counts for
         * @param amount The number of times to increase the count for a given
         * doc_id
         */
        void increase_count(doc_id d_id, uint32_t amount);

        /**
         * @param d_id The document id to query
         * @return the number of times term_id occurred in this postings_data
         */
        uint32_t count(doc_id d_id) const;

        /**
         * @param other The postings_data to compare with
         * @return whether this postings_data is less than (has a smaller
         * term_id than) the parameter
         */
        bool operator<(const postings_data & other) const;

        /**
         * @return the term_id for this postings_data
         */
        term_id term() const;

        /**
         * @return the number of documents that this term occurs in
         */
        uint32_t idf() const;

    private:

        term_id _t_id;
        std::unordered_map<doc_id, uint32_t> _counts;
};

}
}

#endif
