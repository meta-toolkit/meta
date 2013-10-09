/**
 * @file postings_data.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _POSTINGS_DATA_
#define _POSTINGS_DATA_

#include <sstream>
#include <type_traits>
#include <istream>
#include <unordered_map>
#include <string>
#include "io/compressed_file_writer.h"
#include "io/compressed_file_reader.h"
#include "meta.h"

namespace meta {
namespace index {

/**
 * A class to represent the per-PrimaryKey data in an index's postings
 * file. For a given PrimaryKey, a mapping of SecondaryKey -> count information
 * is stored.
 *
 * For example, for an inverted index, PrimaryKey = term_id, SecondaryKey =
 * doc_id. For a forward_index, PrimaryKey = doc_id, SecondaryKey = term_id.
 */
template <class PrimaryKey, class SecondaryKey>
class postings_data
{
    public:

        static_assert(
            (std::is_integral<PrimaryKey>::value ||
             std::is_base_of<util::numeric, PrimaryKey>::value)
            &&
            (std::is_integral<SecondaryKey>::value ||
             std::is_base_of<util::numeric, SecondaryKey>::value
            ),
            "primary and secondary keys in postings data must be numeric types"
        );

        static_assert(
            sizeof(uint64_t) == sizeof(double),
            "sizeof(uint64_t) must equal sizeof(double) since "
            "reinterpret_cast is used in postings_data"
        );

        /**
         * Creates an empty postings_data for a given PrimaryKey.
         * @param p_id The PrimaryKey to be associated with this postings_data
         */
        postings_data(PrimaryKey p_id);

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
         * @param s_id The SecondaryKey's id to add counts for
         * @param amount The number of times to increase the count for a given
         * SecondaryKey
         */
        void increase_count(SecondaryKey s_id, double amount);

        /**
         * @param s_id The SecondaryKey id to query
         * @return the number of times PrimaryKey occurred in this postings_data
         */
        double count(SecondaryKey s_id) const;

        /**
         * @return the per-SecondaryKey frequency information for this
         * PrimaryKey
         */
        const std::unordered_map<SecondaryKey, double> & counts() const;

        /**
         * @param map A map of counts to assign into this postings_data
         */
        void set_counts(const std::unordered_map<SecondaryKey, double> & map);

        /**
         * @param other The postings_data to compare with
         * @return whether this postings_data is less than (has a smaller
         * PrimaryKey than) the parameter
         */
        bool operator<(const postings_data & other) const;

        /**
         * Reads uncompressed postings data from a stream
         * @param in The stream to read from
         * @param pd The postings data object to write the stream info to
         * @return the input stream
         */
        friend std::istream & operator>>(std::istream & in,
                                         postings_data<
                                            PrimaryKey,
                                            SecondaryKey
                                         > & pd)
        {
            std::string buffer;
            std::getline(in, buffer);
            std::istringstream iss{buffer};

            iss >> pd._p_id;
            pd._counts.clear();

            SecondaryKey s_id;
            double count;
            while(iss.good())
            {
                iss >> s_id;
                iss >> count;
                pd._counts[s_id] += count;
            }

            return in;
        }

        /**
         * Writes uncompressed postings data to a stream
         * @param out The stream to write to
         * @param pd The postings data object to write to the stream
         * @return the output stream
         */
        friend std::ostream & operator<<(std::ostream & out,
                                         postings_data<
                                            PrimaryKey,
                                            SecondaryKey
                                         > & pd)
        {
            if(pd._counts.empty())
                return out;

            out << pd._p_id;
            for(auto & p: pd._counts)
            {
                out << " " << p.first;
                out << " " << p.second;
            }
            out << "\n";

            return out;
        }

        /**
         * Writes this postings_data to a compressed file. The mapping for the
         * compressed file is already set, so we don't have to worry about it.
         * We can also assume that we are already in the correct location of the
         * file.
         * @param writer The compressed file to write to
         */
        void write_compressed(io::compressed_file_writer & writer) const;

        /**
         * Reads compressed postings_data into this object. The mapping for the
         * compressed file is already set, so we don't have to worry about it.
         * We can also assume that we are already in the correct location of the
         * file.
         * @param reader The compressed file to read from
         */
        void read_compressed(io::compressed_file_reader & reader);

        /**
         * @return the term_id for this postings_data
         */
        PrimaryKey primary_key() const;

        /**
         * @return the number of SecondaryKeys that this PrimaryKey occurs with
         */
        uint64_t inverse_frequency() const;

    private:

        PrimaryKey _p_id;
        std::unordered_map<SecondaryKey, double> _counts;
        const static uint64_t _delimiter = std::numeric_limits<uint64_t>::max();
};

}
}

#include "index/postings_data.tcc"
#endif
