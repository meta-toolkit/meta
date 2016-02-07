/**
 * @file transition_map.h
 * @author Chase Geigle
 */

#include <cassert>

#include "meta/io/filesystem.h"
#include "meta/io/gzstream.h"
#include "meta/io/packed.h"
#include "meta/parser/transition_map.h"

namespace meta
{
namespace parser
{

transition_map::transition_map(const std::string& prefix)
{
    io::gzifstream store{prefix + "/parser.trans.gz"};
    load(store);
}

void transition_map::load(std::istream& store)
{
    if (!store)
        throw transition_map_exception{"missing transitions model file"};

    uint64_t num_trans;
    io::packed::read(store, num_trans);

    if (!store)
        throw transition_map_exception{"malformed transitions model file"};

    transitions_.reserve(num_trans);
    for (uint64_t i = 0; i < num_trans; ++i)
    {
        if (!store)
            throw transition_map_exception{
                "malformed transition model file (too few "
                "transitions written)"};

        transition::type_t trans_type;
        io::packed::read(store, trans_type);

        util::optional<transition> trans;
        switch (trans_type)
        {
            case transition::type_t::REDUCE_L:
            case transition::type_t::REDUCE_R:
            case transition::type_t::UNARY:
            {
                std::string lbl;
                io::packed::read(store, lbl);
                trans = transition{trans_type, class_label{lbl}};
                break;
            }

            default:
                trans = transition{trans_type};
                break;
        }

        assert(map_.size() <= std::numeric_limits<uint16_t>::max());
        trans_id id{static_cast<uint16_t>(map_.size())};
        map_[*trans] = id;
        transitions_.emplace_back(std::move(*trans));
    }
}

const transition& transition_map::at(trans_id id) const
{
    return transitions_.at(id);
}

trans_id transition_map::at(const transition& trans) const
{
    auto it = map_.find(trans);
    if (it == map_.end())
        throw std::out_of_range{"index out of bounds"};

    return it->second;
}

trans_id transition_map::operator[](const transition& trans)
{
    auto it = map_.find(trans);
    if (it != map_.end())
        return it->second;

    transitions_.push_back(trans);
    assert(map_.size() <= std::numeric_limits<uint16_t>::max());
    trans_id id{static_cast<uint16_t>(map_.size())};
    return map_[trans] = id;
}

uint64_t transition_map::size() const
{
    assert(map_.size() == transitions_.size());
    return map_.size();
}

void transition_map::save(const std::string& prefix) const
{
    io::gzofstream store{prefix + "/parser.trans.gz"};
    io::packed::write(store, transitions_.size());
    for (const auto& trans : transitions_)
    {
        io::packed::write(store, trans.type());
        switch (trans.type())
        {
            case transition::type_t::REDUCE_L:
            case transition::type_t::REDUCE_R:
            case transition::type_t::UNARY:
                io::packed::write(
                    store, static_cast<const std::string&>(trans.label()));
                break;

            default:
                break;
        }
    }
}
}
}
