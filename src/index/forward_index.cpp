/**
 * @file forward_index.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <memory>
#include <unordered_set>
#include "index/forward_index.h"
#include "index/chunk.h"

using std::cerr;
using std::endl;

namespace meta {
namespace index {


forward_index::forward_index(const cpptoml::toml_group & config):
    base{config, *cpptoml::get_as<std::string>(config, "forward-index")}
{ /* nothing */ }

std::string forward_index::liblinear_data(doc_id d_id) const
{
    auto pdata = search_primary(d_id);

    // output the class label (starting with index 1)

    std::stringstream out;
    out << (label_id_from_doc(d_id) + 1);

    // output each term_id:count (starting with index 1)

    using term_pair = std::pair<term_id, double>;
    std::vector<term_pair> sorted;
    sorted.reserve(pdata->counts().size());
    for(auto & p: pdata->counts())
        sorted.emplace_back(std::piecewise_construct,
                            std::forward_as_tuple(p.first + 1),
                            std::forward_as_tuple(p.second));

    std::sort(sorted.begin(), sorted.end(),
        [](const term_pair & a, const term_pair & b) {
            return a.first < b.first;
        }
    );

    for(auto & freq: sorted)
        out << " " << freq.first << ":" << freq.second;

    out << "\n";
    return out.str();
}

void forward_index::chunk_handler::handle_doc(const corpus::document & doc) {
    postings_data<doc_id, term_id> pd{doc.id()};
    pd.set_counts(std::vector<std::pair<term_id, double>>{
        doc.frequencies().begin(), doc.frequencies().end()
    });
    pdata_.push_back(pd);
}

std::vector<postings_data<doc_id, term_id>>
forward_index::chunk_handler::chunk() {
    auto size = pdata_.size();
    auto vec = std::move(pdata_);
    pdata_.reserve(size);
    return vec;
}

forward_index::chunk_handler::~chunk_handler() {
    write_chunk();
}

}
}
