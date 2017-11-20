#include <iostream>
#include <sstream>

#include "optimization.h"

#ifndef TEST_OPT
#define TEST_OPT 1
#endif // TEST_OPT

using namespace meta::stats::opt;

#ifdef TEST_OPT
int main(){
    feature_map<celoe> dm1, dm2, dm3;

    dm1["1"] = 8;
    dm1["2"] = 3;

    dm2["2"] = 4;
    dm2["3"] = 3;

    dm3["3"] = 4;
    dm3["4"] = 6;

    std::vector<feature_map<celoe>> dms;
    dms.push_back(dm1);
    dms.push_back(dm2);
    dms.push_back(dm3);

    dirichlet_optimizer optimizer(dms);

    auto optimal_alpha = optimizer.minka_fpi();

    cout << endl << "optimal alpha: " << optimal_alpha;
}
#else
#include "meta/stats/optimization.h"
#include "meta/analyzers/ngram/ngram_word_analyzer.h"
#include "meta/corpus/document.h"
#include "meta/analyzers/token_stream.h"
#include "meta/analyzers/tokenizers/character_tokenizer.h"

#include "../tests/create_config.h"
#include "meta/meta.h"

#include "../src/analyzers/analyzer.cpp"

using namespace meta::stats::opt;
using namespace meta::analyzers;
using namespace meta::corpus;
using namespace meta::analyzers::tokenizers;
using namespace meta::tests;

std::unique_ptr<token_stream> make_filter() {
    auto line_cfg = create_config("line");
    return default_filter_chain(*line_cfg);
}


int main(){
    document doc1(meta::doc_id{47}), doc2(meta::doc_id{48}), doc3(meta::doc_id{49});
    doc1.content("Quaia Quaia Coronoid");
    doc2.content("Dj extra Quaia Quaia");
    doc3.content("Coronoid Coronoid Diagram Dj");

    character_tokenizer tokenizer;

    tokenizer.set_content("Quaia Quaia Coronoid");

    std::vector<feature_map<celoe>> docs_models;

    ngram_word_analyzer anal(1, make_filter());

    docs_models.push_back(anal.analyze<celoe>(doc1));
    docs_models.push_back(anal.analyze<celoe>(doc2));
    docs_models.push_back(anal.analyze<celoe>(doc3));

    dirichlet_optimizer optimizer(docs_models);

    auto res_map = optimizer.minka_fpi();

    for (auto iter: res_map){
        std::cout << iter.first << " " << iter.second << std::endl;
    }

    return 0;
}
#endif // TEST_OPT
