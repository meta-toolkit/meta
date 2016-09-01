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

#include "meta/config.h"
#include "meta/meta.h"
#include "meta/util/sparse_vector.h"

namespace meta
{
namespace index
{

/**
 * A class to represent the per-PrimaryKey data in an index's postings
 * file. For a given PrimaryKey, a mapping of SecondaryKey -> count information
 * is stored.
 *
 * For example, for an inverted index, PrimaryKey = term_id, SecondaryKey =
 * doc_id. For a forward_index, PrimaryKey = doc_id, SecondaryKey = term_id.
 */
template <class PrimaryKey, class SecondaryKey, class FeatureValue = uint64_t>
class postings_data
{
  public:
    using primary_key_type = PrimaryKey;
    using secondary_key_type = SecondaryKey;
    using pair_t = std::pair<SecondaryKey, FeatureValue>;
    using count_t = std::vector<pair_t>;

    /**
     * PrimaryKeys may only be integral types or strings; SecondaryKeys may
     * only be integral types.
     */
    static_assert(
        (util::is_numeric<PrimaryKey>::value
         || std::is_same<PrimaryKey, std::string>::value)
            && (util::is_numeric<SecondaryKey>::value),
        "primary and secondary keys in postings data must be numeric types");

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
     * Postings data is copy constructable.
     */
    postings_data(const postings_data&) = default;

    /**
     * Postings data is move constructable.
     */
    postings_data(postings_data&&) = default;

    /**
     * Postings data is copy assignable.
     */
    postings_data& operator=(const postings_data&) = default;

    /**
     * Postings data is move assignable.
     */
    postings_data& operator=(postings_data&&) = default;

    /**
     * @param cont The other container (of SecondaryKey, count pairs) to merge
     * Adds the parameter's data to this object's data
     */
    template <class Container>
    void merge_with(Container&& cont);

    /**
     * @param s_id The SecondaryKey's id to add counts for
     * @param amount The number of times to increase the count for a given
     * SecondaryKey
     */
    void increase_count(SecondaryKey s_id, FeatureValue amount);

    /**
     * @param s_id The SecondaryKey id to query
     * @return the number of times SecondaryKey occurred in this
     * postings_data
     */
    FeatureValue count(SecondaryKey s_id) const;

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
     * @param counts A vector of counts to assign into this postings_data
     */
    void set_counts(count_t&& counts);

    /**
     * @param begin The beginning of the counts to assign into this
     * postings_data
     * @param end The end of the counts to assign into this postings_data
     */
    template <class InputIterator>
    void set_counts(InputIterator begin, InputIterator end);

    /**
     * @param other The postings_data to compare with
     * @return whether this postings_data is less than (has a smaller
     * PrimaryKey than) the parameter
     */
    bool operator<(const postings_data& other) const;

    /**
     * Writes this postings data to an output stream in a packed binary
     * format.
     * @param out The stream to write to
     * @return the number of bytes used to write out this postings data
     */
    uint64_t write_packed(std::ostream& out) const;

    /**
     * Writes this postings data's counts to an output stream in a packed
     * binary format.
     * @param out The stream to write to
     * @return the number of bytes used to write out this postings data's
     * counts
     */
    uint64_t write_packed_counts(std::ostream& out) const;

    /**
     * Reads a postings data object from an input stream in a packed binary
     * format.
     * @param in The stream to read from
     * @return the number of bytes read in consuming this postings data
     */
    uint64_t read_packed(std::istream& in);

    /**
     * @return the term_id for this postings_data
     */
    const PrimaryKey& primary_key() const;

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
    util::sparse_vector<SecondaryKey, FeatureValue> counts_;
};

/**
 * @param lhs The first postings_data
 * @param rhs The postings_data to compare with
 * @return whether this postings_data has the same PrimaryKey as
 * the paramter
 */
template <class PrimaryKey, class SecondaryKey, class FeatureValue>
bool operator==(
    const postings_data<PrimaryKey, SecondaryKey, FeatureValue>& lhs,
    const postings_data<PrimaryKey, SecondaryKey, FeatureValue>& rhs);
}
}

namespace std
{
/**
 * Hash specialization for postings_data<PrimaryKey, SecondaryKey,
 * FeatureValue>
 */
template <class PrimaryKey, class SecondaryKey, class FeatureValue>
struct hash<meta::index::postings_data<PrimaryKey, SecondaryKey, FeatureValue>>
{
    using pdata_t
        = meta::index::postings_data<PrimaryKey, SecondaryKey, FeatureValue>;
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

#include "meta/index/postings_data.tcc"
#endif
