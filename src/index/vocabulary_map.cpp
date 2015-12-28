/**
 * @file vocabulary_map.h
 * @author Chase Geigle
 */

#include <cstring>
#include "meta/index/vocabulary_map.h"
#include "meta/util/optional.h"

namespace meta
{
namespace index
{

vocabulary_map::vocabulary_map(const std::string& path, uint16_t block_size)
    : file_{path}, inverse_{path + ".inverse"}, block_size_{block_size}
{
    // determine the position that denotes the end of the leaf node
    // level---we can use this to determine when to stop our finds later on
    auto pos = inverse_[inverse_.size() - 1];
    leaf_end_pos_ = pos + sizeof(term_id) + std::strlen(file_.begin() + pos);

    // determine the position of the first internal node that is not the
    // root---this is useful in find() and we don't want to compute it
    // every time
    auto root_start = file_.size() - block_size_;
    auto first_pos = root_start + std::strlen(file_.begin() + root_start) + 1;
    initial_seek_pos_ = *reinterpret_cast<uint64_t*>(file_.begin() + first_pos);
}

util::optional<term_id> vocabulary_map::find(const std::string& term) const
{
    uint64_t pos = file_.size() - block_size_;
    uint64_t seek_pos = initial_seek_pos_;
    while (pos > leaf_end_pos_)
    {
        uint64_t end_pos = pos + block_size_;
        while (pos < end_pos && *(file_.begin() + pos))
        {
            if (compare(term, file_.begin() + pos) < 0)
            {
                // stale seek pos: this means that the very first term in a
                // block was larger than the target term, meaning we can't find
                // this term in our tree
                if (seek_pos >= pos)
                    return util::nullopt;
                // stop executing this loop: seek_pos determines where we
                // go next
                break;
            }
            else
            {
                // skip over the string
                pos += std::strlen(file_.begin() + pos) + 1;
                // read the position
                seek_pos = *reinterpret_cast<uint64_t*>(file_.begin() + pos);
                pos += sizeof(uint64_t);
            }
        }
        pos = seek_pos; // remember: this decreases pos as we go towards the
                        // leaves
    }

    // we are now at a leaf node---find our term if it exists
    uint64_t endpos = pos + block_size_;
    while (pos < endpos)
    {
        if (compare(term, file_.begin() + pos) == 0)
        {
            pos += term.length() + 1;
            return term_id{*reinterpret_cast<uint64_t*>(file_.begin() + pos)};
        }
        pos += std::strlen(file_.begin() + pos) + 1 + sizeof(uint64_t);
    }
    return util::nullopt;
}

int vocabulary_map::compare(const std::string& term, const char* other) const
{
    return std::memcmp(term.c_str(), other, term.length() + 1);
}

std::string vocabulary_map::find_term(term_id t_id) const
{
    return file_.begin() + inverse_[t_id];
}

uint64_t vocabulary_map::size() const
{
    return inverse_.size();
}
}
}
