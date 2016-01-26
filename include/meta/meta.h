/**
 * @file meta.h
 * Contains top-level namespace documentation for the META toolkit.
 * documentation is included here because there is no main file in each
 * namespace that is a logical choice for such documentation.
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_H_
#define META_H_

#include <cstdint>
#include <string>

#include "meta/util/identifiers.h"
/**
 * The ModErn Text Analysis toolkit is a suite of natural language processing,
 * classification, information retrieval, data mining, and other applications
 * of text processing.
 */
namespace meta
{
/*
 * Represents the name of a class used in classification or feature
 * selection.
 */
MAKE_IDENTIFIER_UDL(class_label, std::string, _cl)

/*
 * Represents a *predicted* class label that is used in classification or
 * features selection; it may not be the true class label
 */
MAKE_IDENTIFIER_UDL(predicted_label, std::string, _pl)

/*
 * Numbering system for string terms.
 */
MAKE_NUMERIC_IDENTIFIER_UDL(term_id, uint64_t, _tid)

/*
 * Numbering system for documents.
 */
MAKE_NUMERIC_IDENTIFIER_UDL(doc_id, uint64_t, _did)

/*
 * Numbering system for class label ids.
 */
MAKE_NUMERIC_IDENTIFIER_UDL(label_id, uint32_t, _lid)

/*
 * Numbering system for query ids.
 */
MAKE_NUMERIC_IDENTIFIER_UDL(query_id, uint64_t, _qid)

/*
 * Numbering system for node ids.
 */
MAKE_NUMERIC_IDENTIFIER_UDL(node_id, uint64_t, _nid)

/**
 * Containers to be used for caching purposes.
 */
namespace caching
{
}

/**
 * Various ways to convert corpus formats into META-readable documents
 */
namespace corpus
{
}

/**
 * Generic learning algorithms and support data structures.
 */
namespace learn
{
}

/**
 * Algorithms for multi-class and binary classification.
 */
namespace classify
{
/**
 * Kernel functions for linear classifiers.
 */
namespace kernel
{
}
/**
 * Loss functions for sgd.
 */
namespace loss
{
}
}

/**
 * Algorithms for regression.
 */
namespace regress
{
}

/**
 * Indexes to create efficient representations of data.
 */
namespace index
{
}

/**
 * Compressed file readers and writers, configuration file
 * readers, a simple parser, and memory-mapped file support.
 */
namespace io
{
/**
 * Parser specifically for libsvm-formatted files.
 */
namespace libsvm_parser
{
}
}

/**
 * Implementation of a thread pool and a parallel for loop.
 */
namespace parallel
{
}

/**
 * Functions for converting to and from various character sets.
 */
namespace utf
{
}

/**
 * Contains various ways to segment text and deal with preprocessed files
 * (POS tags, parse trees, etc).
 */
namespace analyzers
{

/**
 * Contains tokenizers that start off a filter chain.
 */
namespace tokenizers
{
}

/**
 * Contains filters that mutate existing token streams in a filter chain.
 */
namespace filters
{
}

}

/**
 * Topic modeling functionality.
 */
namespace topics
{
}

/**
 * Shared resources and utilities.
 */
namespace util
{
}

/**
 * Contains functions that print to the terminal and provide progress bars.
 */
namespace printing
{
}

/**
 * Contains implementations of the graph data structure and algorithms
 * that operate over them.
 */
namespace graph
{
}

/**
 * Contains implementations of statistical language models.
 */
namespace lm
{
}

/**
 * Contains functions that relate to phrase structure trees and parsing of
 * natural language.
 */
namespace parser
{
}

/**
 * Sequence representations and labeling models/algorithms.
 */
namespace sequence
{
}

/**
 * Probability distributions and other statistics functions.
 */
namespace stats
{
}
}
#endif
