/**
 * @file classify-test.cpp
 */

#include <vector>
#include <string>
#include <iostream>

#include "index/forward_index.h"
#include "index/ranker/all.h"
#include "util/common.h"
#include "util/invertible_map.h"
#include "classify/classifier/all.h"
#include "classify/classifier/dual_perceptron.h"
#include "classify/kernel/polynomial.h"
#include "classify/kernel/radial_basis.h"
#include "classify/kernel/sigmoid.h"

using std::cout;
using std::cerr;
using std::endl;
using namespace meta;

template <class Index, class Classifier>
classify::confusion_matrix cv(Index & idx, Classifier & c)
{
    std::vector<doc_id> docs = idx.docs();
    classify::confusion_matrix matrix;
    auto seconds = common::time<std::chrono::seconds>([&]() {
        matrix = c.cross_validate(docs, 5);
    });
    std::cerr << "time elapsed: " << seconds.count() << "s" << std::endl;
    matrix.print();
    matrix.print_stats();
    return matrix;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " config.toml" << endl;
        return 1;
    }

    auto config = cpptoml::parse_file(argv[1]);
    auto f_idx = index::make_index<index::forward_index, caching::splay_cache>(argv[1]);
 // auto i_idx = index::make_index<index::inverted_index, caching::splay_cache>(argv[1]);

    classify::perceptron p{f_idx};
    classify::winnow w{f_idx};

    auto m1 = cv(f_idx, p);
    auto m2 = cv(f_idx, w);

 // classify::svm_wrapper svm{f_idx,
 //                           *config.get_as<std::string>("liblinear"),
 //                           classify::svm_wrapper::kernel::Quadratic };
 // auto m1 = cv(f_idx, svm);

 // auto kernel_perceptron =
 //     classify::make_perceptron(f_idx, classify::kernel::polynomial{2, 1.0});

 // auto m2 = cv(f_idx, kernel_perceptron);
 // std::cout << "(liblinear vs kernel perceptron) Significant? "
 //           << std::boolalpha
 //           << classify::confusion_matrix::mcnemar_significant( m1, m2 )
 //           << std::endl;

 // classify::linear_svm l2svm{idx};
 // auto m2 = cv(idx, l2svm);
 // std::cout << "(liblinear vs l2svm) Significant? " << std::boolalpha
 //           << classify::confusion_matrix::mcnemar_significant( m1, m2 )
 //           << std::endl;

 // classify::knn<index::okapi_bm25> k{i_idx, 10, 1.5, 0.75, 500.0};
 // auto m2 = cv(i_idx, k);
 // std::cout << "(liblinear vs knn) Significant? " << std::boolalpha
 //           << classify::confusion_matrix::mcnemar_significant( m1, m2 )
 //           << std::endl;

    return 0;
}
