/**
 * @file sequence_extractor.cpp
 * @author Chase Geigle
 */

#include "meta/parser/trees/internal_node.h"
#include "meta/parser/trees/leaf_node.h"
#include "meta/parser/sequence_extractor.h"

namespace meta
{
namespace parser
{

void sequence_extractor::operator()(const leaf_node& ln)
{
    sequence::symbol_t word{static_cast<std::string>(*ln.word())};
    sequence::tag_t tag{static_cast<std::string>(ln.category())};
    seq_.add_observation({word, tag});
}

void sequence_extractor::operator()(const internal_node& in)
{
    in.each_child([&](const node* n)
                  {
                      n->accept(*this);
                  });
}

sequence::sequence sequence_extractor::sequence()
{
    return std::move(seq_);
}
}
}
