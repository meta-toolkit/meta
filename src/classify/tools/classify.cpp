/**
 * @file classify.cpp
 * @author Sean Massung
 * @author Chase Geigle
 */

#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "meta/caching/all.h"
#include "meta/classify/classifier/all.h"
#include "meta/index/forward_index.h"
#include "meta/index/ranker/all.h"
#include "meta/parser/analyzers/tree_analyzer.h"
#include "meta/sequence/analyzers/ngram_pos_analyzer.h"
#include "meta/util/printing.h"
#include "meta/util/progress.h"
#include "meta/util/time.h"

using namespace meta;

template <class Creator>
classify::confusion_matrix cv(Creator&& creator,
                              classify::multiclass_dataset_view docs, bool even)
{
    classify::confusion_matrix matrix;
    auto msec = common::time([&]() {
        matrix = classify::cross_validate(std::forward<Creator>(creator), docs,
                                          5, even);
    });
    std::cerr << "time elapsed: " << msec.count() / 1000.0 << "s" << std::endl;
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
        std::cerr << "Usage:\t" << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();

    // Register additional analyzers
    parser::register_analyzers();
    sequence::register_analyzers();

    auto config = cpptoml::parse_file(argv[1]);
    auto class_config = config->get_table("classifier");
    if (!class_config)
    {
        std::cerr << "Missing classifier configuration group in " << argv[1]
                  << std::endl;
        return 1;
    }

    auto f_idx = index::make_index<index::forward_index>(*config);
    // load the documents into a dataset
    classify::multiclass_dataset dataset{f_idx};

    std::function<std::unique_ptr<classify::classifier>(
        classify::multiclass_dataset_view)>
        creator;
    auto classifier_method = *class_config->get_as<std::string>("method");
    auto even = class_config->get_as<bool>("even-split").value_or(false);
    if (classifier_method == "knn" || classifier_method == "nearest-centroid")
    {
        auto i_idx = index::make_index<index::inverted_index>(*config);
        creator = [=](classify::multiclass_dataset_view fold) {
            return classify::make_classifier(*class_config, std::move(fold),
                                             i_idx);
        };
    }
    else
    {

        creator = [&](classify::multiclass_dataset_view fold) {
            return classify::make_classifier(*class_config, std::move(fold));
        };
    }

    cv(creator, dataset, even);

    return 0;
}
