/**
 * @file path_predict_eval.cpp
 * @author Sean Massung
 */

#include <random>
#include "index/eval/ir_eval.h"
#include "corpus/document.h"
#include "io/parser.h"
#include "classify/classifier/sgd.h"
#include "classify/loss/hinge.h"
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

template <class Index>
std::vector<doc_id> path_predict_eval::partition(const std::vector
                                                 <doc_id>& orig_docs,
                                                 Index& idx)
{
    std::vector<doc_id> pos;
    std::vector<doc_id> neg;
    for (auto& d : orig_docs)
    {
        if (idx->label(d) == class_label{"0"})
            neg.push_back(d);
        else
            pos.push_back(d);
    }

    std::mt19937 gen(1);
    std::shuffle(neg.begin(), neg.end(), gen);
    neg.erase(neg.begin() + pos.size(), neg.end());

    pos.insert(pos.end(), neg.begin(), neg.end());
    return pos;
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
    using namespace classify;

    auto idx = index::make_index<index::memory_forward_index>(config_file_);
    auto config = cpptoml::parse_file(config_file_);

    // split corpus in half

    //auto train_docs = partition(idx->docs(), idx);
    auto train_docs = idx->docs();

    std::mt19937 gen(1);
    std::shuffle(train_docs.begin(), train_docs.end(), gen);
    uint64_t half = train_docs.size() / 2;
    std::vector
        <doc_id> test_docs{train_docs.begin(), train_docs.begin() + half};
    train_docs.erase(train_docs.begin() + half, train_docs.end());

    auto classifier = make_unique
        <sgd>("sgd-model", idx, class_label{"1"}, class_label{"0"},
              make_unique<loss::hinge>());
    classifier->train(train_docs);

    std::cout << "Training on " << train_docs.size() << " docs" << std::endl;

    std::vector<std::string> names;
    io::parser nfile{"pp/pp.mapping", "\n"};
    while (nfile.has_next())
        names.push_back(nfile.next());

    ranks_.clear();
    std::cout << "Testing on " << test_docs.size() << " docs" << std::endl;
    for (auto& d : test_docs)
    {
        auto score = classifier->predict(d);
     // if (score > 0)
     // {
            std::string name = names[d];
            size_t tab = name.find_first_of("\t");
            std::string n1 = name.substr(0, tab);
            std::string n2 = name.substr(tab + 1);
            auto label = idx->label(d);
            ranks_[n1].emplace(n2, score, label);
            ranks_[n2].emplace(n1, score, label);
     // }
    }

    eval_ranks();
}

void path_predict_eval::eval_ranks()
{
    // build results vectors and build qrel file

    std::ofstream qrels{"pp-qrels.txt"};
    std::unordered_map<std::string, doc_id> id_mapping;
    std::vector<std::vector<std::pair<doc_id, double>>> all_results;
    query_id qid{0};
    for (auto& author : ranks_)
    {
        std::vector<std::pair<doc_id, double>> result;
        for (auto& pred : author.second)
        {
            auto it = id_mapping.find(pred.name);
            doc_id did{0};
            if(it == id_mapping.end())
            {
                auto next = id_mapping.size();
                id_mapping[pred.name] = next;
                did = next;
            }
            else
                did = it->second;
            qrels << qid << " " << did << " " << pred.relevance << std::endl;
            result.emplace_back(did, pred.score);
        }
        all_results.push_back(result);
        ++qid;
    }

    // run the evaluation on all the results

    index::ir_eval eval{config_file_};
    qid = 0;
    for(auto& result: all_results)
    {
        eval.print_stats(result, qid); // only look up to 10 docs
        ++qid;
    }

    std::cout << "MAP: " << eval.map() << std::endl;
}
}
}
}
