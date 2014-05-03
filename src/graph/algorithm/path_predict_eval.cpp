/**
 * @file path_predict_eval.cpp
 * @author Sean Massung
 */

#include "graph/algorithm/path_predict_eval.h"

namespace meta
{
namespace graph
{
namespace algorithm
{
path_predict_eval::path_predict_eval(const std::string& config_file)
    : config_file_{config_file}
{
    // nothing
}
void path_predict_eval::predictions()
{
    auto idx = index::make_index<index::memory_forward_index>(config_file_);
    auto config = cpptoml::parse_file(config_file_);
    auto class_config = config.get_group("classifier");
    auto classifier = classify::make_classifier(*class_config, idx);
    auto matrix = classifier->cross_validate(idx->docs(), 10);
    matrix.print();
    matrix.print_stats();
}
void path_predict_eval::rankings()
{
}
}
}
}
