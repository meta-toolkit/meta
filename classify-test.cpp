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
#include "caching/all.h"

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

template <class Index>
void compare_cv(classify::confusion_matrix &, Index &) {
    std::cout << "finished cv comparison!" << std::endl;
}

template <class Index, class Alternative, class... Alternatives>
void compare_cv(classify::confusion_matrix & matrix,
                Index & idx,
                Alternative & alt,
                Alternatives &... alts)
{
    auto m = cv(idx, alt);
    std::cout << "significant: " << std::boolalpha
        << classify::confusion_matrix::mcnemar_significant(matrix, m)
        << std::endl;
    compare_cv(matrix, idx, alts...);
}

template <class Index, class Classifier, class... Alternatives>
void compare_cv(Index & idx, Classifier & c, Alternatives &... alts)
{
    auto matrix = cv(idx, c);
    compare_cv(matrix, idx, alts...);
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " config.toml" << endl;
        return 1;
    }

    auto config = cpptoml::parse_file(argv[1]);
    auto f_idx = index::make_index<index::forward_index, caching::no_evict_cache>(argv[1]);
 // auto i_idx = index::make_index<index::inverted_index, caching::splay_cache>(argv[1]);


    classify::one_vs_all<classify::sgd<classify::loss::hinge>>
    hinge_sgd{f_idx};
    classify::svm_wrapper svm{f_idx,
                              *config.get_as<std::string>("liblinear"),
                              classify::svm_wrapper::kernel::None };

    auto docs = f_idx.docs();
    // load the documents into the cache
    for (size_t i = 0; i < docs.size(); ++i) {
        common::show_progress(i, docs.size(), 1000, "Pre-fetching for cache ");
        f_idx.search_primary(docs[i]);
    }
    common::end_progress("Pre-fetching for cache ");

    // below is a test for rcv1
    //std::vector<doc_id> train{docs.begin(), docs.begin() + 781265};
    //std::vector<doc_id> test{docs.begin() + 781265, docs.end()};

    //std::cout << "Training set size: " << train.size() << std::endl;
    //std::cout << "Testing set size: " << test.size() << std::endl;

    //std::cout << "Training..." << std::endl;
    //auto train_time = common::time<std::chrono::milliseconds>([&]() {
    //    hinge_sgd.train(train);
    //});
    //std::cout << "Took " << train_time.count() / 1000.0 << "s" << std::endl;
    //std::cout << "Testing..." << std::endl;
    //classify::confusion_matrix m;
    //auto test_time = common::time<std::chrono::milliseconds>([&]() {
    //    m = hinge_sgd.test(test);
    //});
    //std::cout << "Took " << test_time.count() / 1000.0 << "s" << std::endl;
    //m.print();
    //m.print_stats();

    classify::winnow w{f_idx};

    //classify::linear_svm l2svm{f_idx};
    //classify::perceptron p{f_idx};
    //classify::one_vs_all<classify::sgd<classify::loss::huber>> huber_sgd{f_idx};
    //classify::one_vs_all<classify::sgd<classify::loss::least_squares>> least_squares_sgd{f_idx};
    classify::one_vs_all<classify::sgd<classify::loss::logistic>> logistic_sgd{f_idx};
    //classify::one_vs_all<classify::sgd<classify::loss::modified_huber>> mod_huber_sgd{f_idx};
    //classify::one_vs_all<classify::sgd<classify::loss::perceptron>> perceptron_sgd{f_idx};
    classify::one_vs_all<classify::sgd<classify::loss::smooth_hinge>> smooth_hinge_sgd{f_idx};
    classify::one_vs_all<classify::sgd<classify::loss::squared_hinge>> squared_hinge_sgd{f_idx};

    compare_cv(f_idx,
               svm,
               hinge_sgd,
               smooth_hinge_sgd,
               squared_hinge_sgd,
               logistic_sgd);

 // classify::svm_wrapper svm{f_idx,
 //                           *config.get_as<std::string>("liblinear"),
 //                           classify::svm_wrapper::kernel::None };
 // auto m1 = cv(f_idx, svm);

 // auto kernel_perceptron =
 //     classify::make_perceptron(f_idx, classify::kernel::polynomial{2, 1.0});

 // auto m2 = cv(f_idx, kernel_perceptron);
 // std::cout << "(liblinear vs kernel perceptron) Significant? "
 //           << std::boolalpha
 //           << classify::confusion_matrix::mcnemar_significant( m1, m2 )
 //           << std::endl;

 // classify::linear_svm l2svm{f_idx};
 // auto m3 = cv(f_idx, l2svm);
 // std::cout << "(sgd perceptron vs l2svm) Significant? " << std::boolalpha
 //           << classify::confusion_matrix::mcnemar_significant( m1, m3 )
 //           << std::endl;

 // classify::knn<index::okapi_bm25> k{i_idx, 10, 1.5, 0.75, 500.0};
 // auto m2 = cv(i_idx, k);
 // std::cout << "(liblinear vs knn) Significant? " << std::boolalpha
 //           << classify::confusion_matrix::mcnemar_significant( m1, m2 )
 //           << std::endl;

    return 0;
}
