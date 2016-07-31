/**
 * @file create_config.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_TESTS_CREATE_CONFIG_H_
#define META_TESTS_CREATE_CONFIG_H_

#include "cpptoml.h"

namespace meta {
namespace tests {

/**
 * @param corpus_type "line" or "file"
 * @param multi Whether to create an array of analyzers to test
 * multi_analyzer
 * @return an analyzer created based on a constructed config file
 */
inline std::shared_ptr<cpptoml::table>
create_config(const std::string& corpus_type, bool multi = false) {
    auto orig_config = cpptoml::parse_file("../config.toml");
    std::ofstream config_file{"test-config.toml"};

    auto stop_words = orig_config->get_as<std::string>("stop-words");
    if (!stop_words)
        throw std::runtime_error{"\"stop-words\" not in config"};

    auto libsvm_modules = orig_config->get_as<std::string>("libsvm-modules");
    if (!libsvm_modules)
        throw std::runtime_error{"\"libsvm-modules\" not in config"};

    auto punctuation = orig_config->get_as<std::string>("punctuation");
    if (!punctuation)
        throw std::runtime_error{"\"punctuation\" not in config"};

    auto start_exeptions = orig_config->get_as<std::string>("start-exceptions");
    if (!start_exeptions)
        throw std::runtime_error{"\"start-exceptions\" not in config"};

    auto end_exceptions = orig_config->get_as<std::string>("end-exceptions");
    if (!end_exceptions)
        throw std::runtime_error{"\"end-exceptions\" not in config"};

    auto table = cpptoml::make_table();
    table->insert("stop-words", *stop_words);
    table->insert("punctuation", *punctuation);
    table->insert("start-exceptions", *start_exeptions);
    table->insert("end-exceptions", *end_exceptions);
    table->insert("prefix", *orig_config->get_as<std::string>("prefix"));
    table->insert("query-judgements", "../data/ceeaus-qrels.txt");
    table->insert("libsvm-modules", *libsvm_modules);
    table->insert("dataset", "ceeaus");
    table->insert("corpus", corpus_type + ".toml");
    table->insert("encoding", "shift_jis");
    table->insert("index", "ceeaus");

    auto anas = cpptoml::make_table_array();
    auto ana = cpptoml::make_table();
    ana->insert("method", "ngram-word");
    ana->insert<int64_t>("ngram", 1);
    ana->insert("filter", "default-chain");
    anas->push_back(ana);

    if (multi) {
        auto ana = cpptoml::make_table();
        ana->insert("method", "ngram-word");
        ana->insert<int64_t>("ngram", 3);
        ana->insert("filter", "default-chain");
        anas->push_back(ana);
    }

    table->insert("analyzers", anas);

    auto lm = cpptoml::make_table();
    lm->insert("arpa-file", "../data/english-sentences.arpa");
    lm->insert("binary-file-prefix", "test-lm-");

    table->insert("language-model", lm);

    return table;
}
}
}
#endif
