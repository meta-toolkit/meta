/**
 * @file classify-test.cpp
 */

#include <iostream>
#include <string>
#include <vector>

#include "caching/all.h"
#include "classify/classifier/all.h"
#include "classify/loss/all.h"
#include "index/forward_index.h"
#include "index/ranker/all.h"
#include "util/printing.h"
#include "util/progress.h"
#include "util/time.h"

using std::cout;
using std::cerr;
using std::endl;
using namespace meta;

template <class Index, class Classifier>
classify::confusion_matrix cv(Index& idx, Classifier& c, bool even)
{
    std::vector<doc_id> docs = idx.docs();
    classify::confusion_matrix matrix;
    auto seconds = common::time<std::chrono::seconds>(
        [&]()
        {
            matrix = c.cross_validate(docs, 5, even);
        });
    std::cerr << "time elapsed: " << seconds.count() << "s" << std::endl;
    matrix.print();
    matrix.print_stats();
    return matrix;
}

template <class Index>
void compare_cv(classify::confusion_matrix&, Index&)
{
    std::cout << "finished cv comparison!" << std::endl;
}

template <class Index, class Alternative, class... Alternatives>
void compare_cv(classify::confusion_matrix& matrix, Index& idx,
                Alternative& alt, Alternatives&... alts)
{
    auto m = cv(idx, alt);
    std::cout << "significant: " << std::boolalpha
              << classify::confusion_matrix::mcnemar_significant(matrix, m)
              << std::endl;
    compare_cv(matrix, idx, alts...);
}

template <class Index, class Classifier, class... Alternatives>
void compare_cv(Index& idx, Classifier& c, Alternatives&... alts)
{
    auto matrix = cv(idx, c);
    compare_cv(matrix, idx, alts...);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " config.toml" << endl;
        return 1;
    }

    logging::set_cerr_logging();
    auto config = cpptoml::parse_file(argv[1]);
    auto class_config = config.get_table("classifier");
    if (!class_config)
    {
        cerr << "Missing classifier configuration group in " << argv[1] << endl;
        return 1;
    }

    auto f_idx = index::make_index<index::memory_forward_index>(argv[1]);

    auto docs = f_idx->docs();
    printing::progress progress{" > Pre-fetching for cache: ", docs.size()};
    // load the documents into the cache
    for (size_t i = 0; i < docs.size(); ++i)
    {
        progress(i);
        f_idx->search_primary(docs[i]);
    }
    progress.end();

    std::unique_ptr<classify::classifier> classifier;
    auto classifier_method = *class_config->get_as<std::string>("method");
    if (classifier_method == "knn" || classifier_method == "nearest-centroid")
    {
        auto i_idx
            = index::make_index<index::dblru_inverted_index>(argv[1], 10000);
        classifier = classify::make_classifier(*class_config, f_idx, i_idx);
    }
    else
        classifier = classify::make_classifier(*class_config, f_idx);

    bool even = false;
    auto even_split = class_config->get_as<std::string>("even-split");
    if (even_split && *even_split == "true")
        even = true;
    cv(*f_idx, *classifier, even);

    return 0;
}
