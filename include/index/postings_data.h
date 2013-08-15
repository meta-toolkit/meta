/**
 * @file postings_data.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _POSTINGS_DATA_
#define _POSTINGS_DATA_

#include <istream>
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
         * @param other The other postings_data object to consume
         * Adds the parameter's data to this object's data
         */
        void merge_with(const postings_data & other);

        /**
         * @param d_id The doc_id of the new document to add counts for
         * @param amount The number of times to increase the count for a given
         * doc_id
         */
        void increase_count(doc_id d_id, uint64_t amount);

        /**
         * @param d_id The document id to query
         * @return the number of times term_id occurred in this postings_data
         */
        uint64_t count(doc_id d_id) const;

        /**
         * @param other The postings_data to compare with
         * @return whether this postings_data is less than (has a smaller
         * term_id than) the parameter
         */
        bool operator<(const postings_data & other) const;

        /**
         * @param in The stream to read from
         * @param pd The postings data object to write the stream info to
         * @return the input stream
         */
        friend std::istream & operator>>(std::istream & in, postings_data & pd);

        /**
         * @param out The stream to write to
         * @param pd The postings data object to write to the stream
         * @return the output stream
         */
        friend std::ostream & operator<<(std::ostream & out, postings_data & pd);

        /**
         * @return the term_id for this postings_data
         */
        term_id term() const;

        /**
         * @return the number of documents that this term occurs in
         */
        uint64_t idf() const;

    private:

        term_id _t_id;
        std::unordered_map<doc_id, uint64_t> _counts;
};

}
}

#endif
