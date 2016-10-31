//
// Created by Collin Gress on 10/30/16.
//

#include <iostream>
#include "meta/index/inverted_index.h"

using namespace meta;

int main(int argc, char* argv[])
{
    auto config = cpptoml::parse_file(argv[1]);
    auto idx = index::make_index<index::dblru_inverted_index>(*config, 30000);

    std::string test_string = "this is a test string. the vector is populated by the words in this test string";
    corpus::document d;
    d.content(test_string);

    // note that when tokenized, "default-unigram-chain" filtering is done
    auto counts = idx->tokenize(d);
    d.vector().from_feature_map(counts, *idx);

    return 0;
}