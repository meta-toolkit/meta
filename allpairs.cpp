/**
 * @file allpairs.cpp
 * @author Sean Massung
 */

#include <fstream>
#include <iostream>
#include <numeric>
#include "corpus/corpus.h"
#include "index/forward_index.h"
#include "index/postings_data.h"
#include "util/filesystem.h"
#include "util/mapping.h"
#include "analyzers/analyzer.h"
#include "logging/logger.h"
#include "parallel/parallel_for.h"
#include "cpptoml.h"

using namespace meta;

template <class Doc>
double cosine(const Doc& one, const Doc& two, double one_size, double two_size)
{
    double num = 0.0;
    for (auto& term : one->counts())
        num += term.second * two->count(term.first);

    return num / (one_size * two_size);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();
    auto idx = index::make_index<index::forward_index>(argv[1]);

    std::ofstream out{"similarity.cosine"};
    std::mutex mtx;

    uint64_t num_docs = idx->num_docs();
    std::vector<double> sizes;
    sizes.reserve(num_docs);
    for (doc_id i{0}; i < num_docs; ++i)
    {
        double size = 0.0;
        auto terms = idx->search_primary(i)->counts();
        for (auto& term : terms)
            size += term.second * term.second;
        sizes.push_back(sqrt(size));
    }

    printing::progress prog{"Calculating similarities ",
                            num_docs*(num_docs - 1), 1000};
    std::vector<doc_id> ids(num_docs, doc_id{0});
    std::iota(ids.begin(), ids.end(), 0);
    size_t done = 0;
    parallel::parallel_for(ids.begin(), ids.end(), [&](doc_id i)
    {
        auto one = idx->search_primary(i);
        for (doc_id j{i + 1}; j < num_docs; ++j)
        {
            auto two = idx->search_primary(j);
            double score = cosine(one, two, sizes[i], sizes[j]);
            std::lock_guard<std::mutex> lock{mtx};
            prog(done++);
            out << i << " " << j << " " << score << std::endl;
        }
    });
}
