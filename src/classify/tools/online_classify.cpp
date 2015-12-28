/**
 * @file online_classify.cpp
 * @author Chase Geigle
 */

#include <iostream>

#include "meta/classify/batch_training.h"
#include "meta/classify/classifier_factory.h"
#include "meta/classify/classifier/online_classifier.h"
#include "meta/logging/logger.h"
#include "meta/parser/analyzers/tree_analyzer.h"
#include "meta/sequence/analyzers/ngram_pos_analyzer.h"
#include "meta/util/time.h"

int main(int argc, char* argv[])
{
    using namespace meta;

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

    auto batch_size = config->get_as<int64_t>("batch-size");
    if (!batch_size)
    {
        std::cerr << "Missing batch-size in " << argv[1] << std::endl;
        return 1;
    }

    auto test_start = config->get_as<int64_t>("test-start");
    if (!test_start)
    {
        std::cerr << "Missing test-start in " << argv[1] << std::endl;
        return 1;
    }

    auto f_idx = index::make_index<index::forward_index>(*config);

    if (static_cast<uint64_t>(*test_start) > f_idx->num_docs())
    {
        std::cerr << "The start of the test set is more than the number of "
                     "docs in the index."
                  << std::endl;
        return 1;
    }

    // construct the classifier using an empty dataset: this is a trick to
    // ensure that the classifier gets the relevant metadata it needs for
    // setting up e.g. its weight vector
    auto none = util::range(0_did, 0_did);
    classify::multiclass_dataset empty{f_idx, none.end(), none.end()};
    auto classifier = classify::make_classifier(*class_config, empty);

    auto online_classifier
        = dynamic_cast<classify::online_classifier*>(classifier.get());

    if (!online_classifier)
    {
        std::cerr << "The classifier you've chosen ("
                  << *class_config->get_as<std::string>("method")
                  << ") does not support online classification" << std::endl;
        return 1;
    }

    auto docs = f_idx->docs();
    auto test_begin = docs.begin() + *test_start;

    std::vector<doc_id> training_set{docs.begin(), test_begin};
    std::vector<doc_id> test_set{test_begin, docs.end()};

    auto dur = common::time(
        [&]()
        {
            classify::batch_train(f_idx, *online_classifier, training_set,
                                  static_cast<uint64_t>(*batch_size));

            classify::multiclass_dataset test_data{f_idx, test_set.begin(),
                                                   test_set.end()};

            auto mtrx = classifier->test(test_data);
            mtrx.print();
            mtrx.print_stats();
        });

    std::cout << "Took " << dur.count() / 1000.0 << "s" << std::endl;

    return 0;
}
