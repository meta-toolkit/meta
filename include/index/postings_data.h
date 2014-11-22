/**
 * @file postings_data.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_POSTINGS_DATA_
#define META_POSTINGS_DATA_

#include <fstream>
#include <limits>
#include <utility>
#include <vector>

#include "meta.h"
#include "io/compressed_file_reader.h"
#include "io/compressed_file_writer.h"
#include "util/sparse_vector.h"

namespace meta
{
namespace index
{

template <class, class>
class postings_data;

template <class PrimaryKey, class SecondaryKey>
io::compressed_file_reader& operator>>(io::compressed_file_reader&,
                                       postings_data<PrimaryKey,
                                                     SecondaryKey>&);

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
    using primary_key_type = PrimaryKey;
    using secondary_key_type = SecondaryKey;
    using pair_t = std::pair<SecondaryKey, double>;
    using count_t = std::vector<pair_t>;

    /**
     * PrimaryKeys may only be integral types or strings; SecondaryKeys may
     * only be integral types.
     */
    static_assert(
        (std::is_integral<PrimaryKey>::value
         || std::is_base_of<util::numeric, PrimaryKey>::value
         || std::is_same<PrimaryKey, std::string>::value)
        &&
        (std::is_integral<SecondaryKey>::value
         || std::is_base_of<util::numeric, SecondaryKey>::value),
        "primary and secondary keys in postings data must be numeric types");

    /**
     * uint64_t and double must take up the same number of bytes since they are
     * being casted to each other when compressing.
     */
    static_assert(sizeof(uint64_t) == sizeof(double),
                  "sizeof(uint64_t) must equal sizeof(double) since "
                  "reinterpret_cast is used in postings_data");

    /**
     * postings_data is default-constructable.
     */
    postings_data() = default;

    /**
     * Creates an empty postings_data for a given PrimaryKey.
     * @param p_id The PrimaryKey to be associated with this postings_data
     */
    postings_data(PrimaryKey p_id);

    /**
     * @param other The other postings_data object to consume
     * Adds the parameter's data to this object's data
     */
    void merge_with(postings_data& other);

    /**
     * @param s_id The SecondaryKey's id to add counts for
     * @param amount The number of times to increase the count for a given
     * SecondaryKey
     */
    void increase_count(SecondaryKey s_id, double amount);

    /**
     * @param s_id The SecondaryKey id to query
     * @return the number of times SecondaryKey occurred in this
     * postings_data
     */
    double count(SecondaryKey s_id) const;

    /**
     * @return the per-SecondaryKey frequency information for this
     * PrimaryKey
     */
    const count_t& counts() const;

    /**
     * @param counts A map of counts to assign into this postings_data
     */
    void set_counts(const count_t& counts);

    /**
     * @param other The postings_data to compare with
     * @return whether this postings_data is less than (has a smaller
     * PrimaryKey than) the parameter
     */
    bool operator<(const postings_data& other) const;

    /**
     * Helper function used by istream operator.
     * @param in The stream to read from
     * @param pd The postings data object to write the stream info to
     */
    friend void stream_helper(io::compressed_file_reader& in,
                              postings_data<PrimaryKey, SecondaryKey>& pd)
    {
        pd.counts_.clear();
        uint32_t num_pairs = in.next();
        for (uint32_t i = 0; i < num_pairs; ++i)
        {
            SecondaryKey s_id = SecondaryKey{in.next()};
            uint64_t count = in.next();
            pd.counts_.emplace_back(s_id, static_cast<double>(count));
        }
    }

    /**
     * Reads semi-compressed postings data from a compressed file.
     * @param in The stream to read from
     * @param pd The postings data object to write the stream info to
     * @return the input stream
     */
    friend io::compressed_file_reader& operator>>
        <>(io::compressed_file_reader& in,
           postings_data<PrimaryKey, SecondaryKey>& pd);

    /**
     * Writes semi-compressed postings data to a compressed file.
     * @param out The stream to write to
     * @param pd The postings data object to write to the stream
     * @return the output stream
     */
    friend io::compressed_file_writer& operator<<(
        io::compressed_file_writer& out,
        const postings_data<PrimaryKey, SecondaryKey>& pd)
    {
        if (pd.counts_.empty())
            return out;

        out.write(pd.p_id_);
        uint32_t size = pd.counts_.size();
        out.write(size);
        for (auto& p : pd.counts_)
        {
            out.write(p.first);
            out.write(p.second);
        }

        return out;
    }

    /**
     * Writes this postings_data to a compressed file. The mapping for the
     * compressed file is already set, so we don't have to worry about it.
     * We can also assume that we are already in the correct location of the
     * file.
     * @param writer The compressed file to write to
     */
    void write_compressed(io::compressed_file_writer& writer) const;

    /**
     * Reads compressed postings_data into this object. The mapping for the
     * compressed file is already set, so we don't have to worry about it.
     * We can also assume that we are already in the correct location of the
     * file.
     * @param reader The compressed file to read from
     */
    void read_compressed(io::compressed_file_reader& reader);

    /**
     * @param out The output stream to write to
     */
    void write_libsvm(std::ofstream& out) const
    {
        out << p_id_;
        for (auto& c : counts_)
            out << ' ' << (c.first + 1) << ':' << c.second;
        out << '\n';
    }

    /**
     * @return the term_id for this postings_data
     */
    PrimaryKey primary_key() const;

    /**
     * @param new_key
     */
    void set_primary_key(PrimaryKey new_key);

    /**
     * @return the number of SecondaryKeys that this PrimaryKey occurs with
     */
    uint64_t inverse_frequency() const;

    /**
     * @return the number of bytes used for this postings_data
     */
    uint64_t bytes_used() const;

  private:
    /// Primary id this postings_data represents
    PrimaryKey p_id_;

    /// The (secondary_key_type, count) pairs
    util::sparse_vector<SecondaryKey, double> counts_;

    /// delimiter used when writing to compressed files
    const static uint64_t delimiter_ = std::numeric_limits<uint64_t>::max();
};

/**
 * Reads semi-compressed postings data from a compressed file.
 * @param in The stream to read from
 * @param pd The postings data object to write the stream info to
 * @return the input stream
 */
template <class PrimaryKey, class SecondaryKey>
io::compressed_file_reader& operator>>(io::compressed_file_reader& in,
                                       postings_data<PrimaryKey,
                                                     SecondaryKey>& pd)
{
    pd.p_id_ = in.next();
    stream_helper(in, pd);
    return in;
}

/**
 * Reads semi-compressed postings data from a compressed file.
 * @param in The stream to read from
 * @param pd The postings data object to write the stream info to
 * @return the input stream
 */
template <>
inline io::compressed_file_reader& operator>>
    <>(io::compressed_file_reader& in, postings_data<std::string, doc_id>& pd)
{
    pd.p_id_ = in.next_string();
    stream_helper(in, pd);
    return in;
}

/**
 * @param lhs The first postings_data
 * @param rhs The postings_data to compare with
 * @return whether this postings_data has the same PrimaryKey as
 * the paramter
 */
template <class PrimaryKey, class SecondaryKey>
bool operator==(const postings_data<PrimaryKey, SecondaryKey>& lhs,
                const postings_data<PrimaryKey, SecondaryKey>& rhs);
}
}

namespace std
{
template <class PrimaryKey, class SecondaryKey>
/**
 * Hash specialization for postings_data<PrimaryKey, SecondaryKey>
 */
struct hash<meta::index::postings_data<PrimaryKey, SecondaryKey>>
{
    using pdata_t = meta::index::postings_data<PrimaryKey, SecondaryKey>;
    /**
     * @param pd The postings_data to hash
     * @return the hash of the given postings_data
     */
    size_t operator()(const pdata_t& pd) const
    {
        return std::hash<PrimaryKey>{}(pd.primary_key());
    }
};
}

#include "index/postings_data.tcc"
#endif
