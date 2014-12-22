/**
 * @file transition_map.h
 * @author Chase Geigle
 */

#include <cassert>
#include <fstream>

#include "io/binary.h"
#include "parser/transition_map.h"

namespace meta
{
namespace parser
{

transition_map::transition_map(const std::string& prefix)
{
    std::ifstream store{prefix + "/parser.trans", std::ios::binary};

    size_t num_trans;
    io::read_binary(store, num_trans);

    if (!store)
        throw exception{"malformed transitions model file"};

    transitions_.reserve(num_trans);
    for (size_t i = 0; i < num_trans; ++i)
    {
        if (!store)
            throw exception{"malformed transition model file (too few "
                            "transitions written)"};

        int trans_type;
        io::read_binary(store, trans_type);

        util::optional<transition> trans;
        if (trans_type == 0)
        {
            trans = transition{transition::type_t::SHIFT};
        }
        else if (trans_type == 1)
        {
            class_label lbl;
            io::read_binary(store, lbl);
            trans = transition{transition::type_t::REDUCE_L, lbl};
        }
        else if (trans_type == 2)
        {
            class_label lbl;
            io::read_binary(store, lbl);
            trans = transition{transition::type_t::REDUCE_R, lbl};
        }
        else if (trans_type == 3)
        {
            class_label lbl;
            io::read_binary(store, lbl);
            trans = transition{transition::type_t::UNARY, lbl};
        }
        else if (trans_type == 4)
        {
            trans = transition{transition::type_t::FINALIZE};
        }
        else if (trans_type == 5)
        {
            trans = transition{transition::type_t::IDLE};
        }
        else
        {
            throw exception{"invalid transition identifier in model file"};
        }

        auto id = static_cast<trans_id>(map_.size());
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

transition& transition_map::operator[](trans_id id)
{
    return transitions_.at(id);
}

trans_id transition_map::operator[](const transition& trans)
{
    auto it = map_.find(trans);
    if (it != map_.end())
        return it->second;

    transitions_.push_back(trans);
    auto id = static_cast<trans_id>(map_.size());
    return map_[trans] = id;
}

uint64_t transition_map::size() const
{
    assert(map_.size() == transitions_.size());
    return map_.size();
}

void transition_map::save(const std::string& prefix) const
{
    std::ofstream store{prefix + "/parser.trans", std::ios::binary};

    io::write_binary(store, transitions_.size());
    for (const auto& trans : transitions_)
    {
        switch (trans.type())
        {
            case transition::type_t::SHIFT:
                io::write_binary(store, 0);
                break;

            case transition::type_t::REDUCE_L:
                io::write_binary(store, 1);
                io::write_binary(store, trans.label());
                break;

            case transition::type_t::REDUCE_R:
                io::write_binary(store, 2);
                io::write_binary(store, trans.label());
                break;

            case transition::type_t::UNARY:
                io::write_binary(store, 3);
                io::write_binary(store, trans.label());
                break;

            case transition::type_t::FINALIZE:
                io::write_binary(store, 4);
                break;

            case transition::type_t::IDLE:
                io::write_binary(store, 5);
                break;
        }
    }
}
}
}
