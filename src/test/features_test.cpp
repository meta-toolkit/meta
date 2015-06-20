/**
 * @file features_test.cpp
 * @author Sean Massung
 */

#include "test/features_test.h"
#include "test/inverted_index_test.h"
#include "features/feature_selector.h"
#include "features/selector_factory.h"
#include "features/all.h"

namespace meta
{
namespace testing
{

namespace
{
template <class Index>
void test_construction(Index& idx, const std::string& id)
{
    std::ofstream fconfig{"feature-config.toml"};
    fconfig << "[features]\nmethod = \"" << id << "\"\n"
            << "prefix = \"test-features\"";
    fconfig.close();
    auto config = cpptoml::parse_file("feature-config.toml");
    auto selector = features::make_selector(config, idx);
    selector->select(20);
    selector->select(50);
    selector->select_percent(0.05);
    selector->select_percent(0.10);

    auto t_id = idx->get_term_id("china"); // this term should be selected
    ASSERT(selector->selected(t_id));

    // CEEAUS has three classes
    ASSERT(filesystem::file_exists("test-features." + id + ".1"));
    ASSERT(filesystem::file_exists("test-features." + id + ".2"));
    ASSERT(filesystem::file_exists("test-features." + id + ".3"));
    ASSERT(filesystem::file_exists("test-features." + id + ".selected"));
}
}

int features_tests()
{
    int failed = 0;
    create_config("line");

    // scope for forward index object
    {
        auto f_idx = index::make_index<index::memory_forward_index>(
            "test-config.toml");

        failed += testing::run_test("chi-square", [&]()
                                    {
                                        test_construction(f_idx, "chi-square");
                                    });
        failed += testing::run_test("info-gain", [&]()
                                    {
                                        test_construction(f_idx, "info-gain");
                                    });
        failed += testing::run_test("corr-coef", [&]()
                                    {
                                        test_construction(f_idx, "corr-coef");
                                    });
    }

    system("rm -rf ceeaus-* test-features.*");
    filesystem::delete_file("feature-config.toml");
    return failed;
}
}
}
