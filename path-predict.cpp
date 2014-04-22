/**
 * @file path-predict.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <queue>
#include <vector>
#include "corpus/document.h"
#include "graph/directed_graph.h"
#include "graph/algorithm/metapath_measures.h"
#include "graph/dblp_node.h"
#include "graph/dblp_loader.h"
#include "logging/logger.h"

using namespace meta;

/**
 * @param g The time-sliced DBLP network
 * @param metapath The metapath to use for feature generation
 * @return positive and negative documents representing pairs of
 * potentially-collaborating authors
 */
template <class Graph>
std::vector<corpus::document> create_docs(Graph& g, const std::vector
                                          <std::string>& metapath)
{
    std::vector<corpus::document> docs;
    return docs;
}

/**
 * Creates a libsvm-formatted corpus for use with any MeTA classifiers
 * @param train The training documents
 * @param test The testing documents
 */
void write_dataset(const std::vector<corpus::document>& train,
                   const std::vector<corpus::document>& test)
{
}

int main(int argc, char* argv[])
{
    logging::set_cerr_logging();
    std::string prefix = "/home/sean/projects/meta-data/dblp/"; // testing

    graph::directed_graph<graph::dblp_node> g_train;
    graph::directed_graph<graph::dblp_node> g_test;

    std::vector<std::string> metapath = {"author", "paper", "author"};

    graph::dblp_loader::load(g_train, prefix, 0, 2005);
    std::vector<corpus::document> train_docs = create_docs(g_train, metapath);

    graph::dblp_loader::load(g_test, prefix, 2006, 2012);
    std::vector<corpus::document> test_docs = create_docs(g_test, metapath);

    write_dataset(train_docs, test_docs);
    std::cout << "Train: doc_id [0, " << train_docs.size() - 1 << "]"
              << std::endl;
    std::cout << "Test: doc_id [" << train_docs.size() << ", "
              << train_docs.size() + test_docs.size() - 1 << "]" << std::endl;
}
