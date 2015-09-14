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
    auto config = cpptoml::make_table();
    auto fcfg = cpptoml::make_table();
    fcfg->insert("method", id);
    fcfg->insert("prefix", "test-features");
    config->insert("features", fcfg);

    auto selector = features::make_selector(*config, idx);
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
    auto line_cfg = create_config("line");

    // scope for forward index object
    {
        auto f_idx = index::make_index<index::memory_forward_index>(*line_cfg);

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
        failed += testing::run_test("odds-ratio", [&]()
                                    {
                                        test_construction(f_idx, "odds-ratio");
                                    });
    }

    filesystem::remove_all("ceeaus-inv");
    filesystem::remove_all("ceeaus-fwd");
    for (const std::string& id :
         {"chi-square", "info-gain", "corr-coef", "odds-ratio"})
    {
        for (const std::string& suffix : {"1", "2", "3", "selected"})
        {
            filesystem::remove_all("test-features." + id + "." + suffix);
        }
    }
    return failed;
}
}
}
