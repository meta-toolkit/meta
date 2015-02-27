/**
 * @file online-classify.cpp
 */

#include <iostream>
#include "classify/batch_training.h"
#include "classify/classifier_factory.h"
#include "logging/logger.h"
#include "util/time.h"

int main(int argc, char* argv[])
{
    using namespace meta;

    if (argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " config.toml" << std::endl;
        return 1;
    }

    logging::set_cerr_logging();
    auto config = cpptoml::parse_file(argv[1]);
    auto class_config = config.get_table("classifier");
    if (!class_config)
    {
        std::cerr << "Missing classifier configuration group in " << argv[1]
                  << std::endl;
        return 1;
    }

    auto batch_size = config.get_as<int64_t>("batch-size");
    if (!batch_size)
    {
        std::cerr << "Missing batch-size in " << argv[1] << std::endl;
        return 1;
    }

    auto test_start = config.get_as<int64_t>("test-start");
    if (!test_start)
    {
        std::cerr << "Missing test-start in " << argv[1] << std::endl;
        return 1;
    }

    auto f_idx = index::make_index<index::memory_forward_index>(argv[1]);

    if (static_cast<uint64_t>(*test_start) > f_idx->num_docs())
    {
        std::cerr << "The start of the test set is more than the number of "
                     "docs in the index." << std::endl;
        return 1;
    }

    auto classifier = classify::make_classifier(*class_config, f_idx);

    auto docs = f_idx->docs();
    auto test_begin = docs.begin() + *test_start;

    std::vector<doc_id> training_set{docs.begin(), test_begin};
    std::vector<doc_id> test_set{test_begin, docs.end()};

    auto dur = common::time([&]()
    {
        classify::batch_train(*f_idx, *classifier, training_set, *batch_size);

        auto mtrx = classifier->test(test_set);
        mtrx.print();
        mtrx.print_stats();
    });

    std::cout << "Took "
              << std::chrono::duration_cast<std::chrono::seconds>(dur).count()
              << "s" << std::endl;

    return 0;
}
