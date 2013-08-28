/**
 * @file classify-test.cpp
 */

#include <vector>
#include <string>
#include <iostream>

#include "index/forward_index.h"
#include "util/common.h"
#include "util/invertible_map.h"
#include "classify/classifier/all.h"

using std::cout;
using std::cerr;
using std::endl;
using namespace meta;

classify::confusion_matrix cv(const std::unique_ptr<index::forward_index> & idx,
                              classify::classifier & c)
{
    std::vector<doc_id> docs = idx->docs();
    classify::confusion_matrix matrix = c.cross_validate(docs, 5);
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

    auto config = common::read_config(argv[1]);
    std::unique_ptr<index::forward_index> idx = index::forward_index::load_index(config);

    //classify::liblinear_svm svm{ idx, *cpptoml::get_as<std::string>( config, "liblinear" ) };
    //auto m1 = cv(idx, svm);

    classify::linear_svm l2svm{idx};
    auto m2 = cv(idx, l2svm);
 // std::cout << "(liblinear vs l2svm) Significant? " << std::boolalpha
 //           << classify::confusion_matrix::mcnemar_significant( m1, m2 )
 //           << std::endl;

    classify::linear_svm l1svm{ idx, classify::linear_svm::loss_function::L1 };
    auto m3 = cv(idx, l1svm);
    std::cout << "(liblinear vs l1svm) Significant? " << std::boolalpha
              << classify::confusion_matrix::mcnemar_significant( m2, m3 )
              << std::endl;

 // classify::perceptron p{idx};
 // auto m4 = cv(idx, p);
 // std::cout << "(liblinear vs perceptron) Significant? " << std::boolalpha
 //           << classify::confusion_matrix::mcnemar_significant( m2, m4 )
 //           << std::endl;

 // classify::naive_bayes nb{idx};
 // auto m5 = cv(idx, nb);
 // std::cout << "(liblinear vs naive bayes) Significant? " << std::boolalpha
 //           << classify::confusion_matrix::mcnemar_significant( m2, m5 )
 //           << std::endl;

    return 0;
}
