/**
 * @file feature_selection_test.cpp
 * @author Sean Massung
 */

#include "bandit/bandit.h"
#include "create_config.h"
#include "features/feature_selector.h"
#include "features/selector_factory.h"
#include "features/all.h"

using namespace bandit;
using namespace meta;

namespace {

template <class Index>
void test_method(Index& idx, const std::string& id) {
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
    AssertThat(selector->selected(t_id), IsTrue());

    // CEEAUS has three classes
    AssertThat(filesystem::file_exists("test-features." + id + ".1"), IsTrue());
    AssertThat(filesystem::file_exists("test-features." + id + ".2"), IsTrue());
    AssertThat(filesystem::file_exists("test-features." + id + ".3"), IsTrue());
    AssertThat(filesystem::file_exists("test-features." + id + ".selected"),
               IsTrue());
}
}

go_bandit([]() {
    auto line_cfg = tests::create_config("line");
    auto f_idx = index::make_index<index::memory_forward_index>(*line_cfg);

    // run each test twice to ensure files can be read from disk
    describe("[feature-selection]", [&]() {

        it("should implement chi square", [&]() {
            test_method(f_idx, "chi-square");
            test_method(f_idx, "chi-square");
        });

        it("should implement information gain", [&]() {
            test_method(f_idx, "info-gain");
            test_method(f_idx, "info-gain");
        });

        it("should implement correlation coefficient", [&]() {
            test_method(f_idx, "corr-coef");
            test_method(f_idx, "corr-coef");
        });

        it("should implement odds ratio", [&]() {
            test_method(f_idx, "odds-ratio");
            test_method(f_idx, "odds-ratio");
        });
    });

    f_idx = nullptr;
    filesystem::remove_all("ceeaus-inv");
    filesystem::remove_all("ceeaus-fwd");
    for (const std::string& id :
         {"chi-square", "info-gain", "corr-coef", "odds-ratio"}) {
        for (const std::string& suffix : {"1", "2", "3", "selected"}) {
            filesystem::remove_all("test-features." + id + "." + suffix);
        }
    }
});
