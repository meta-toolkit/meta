/**
 * @file meta.h
 * Contains default namespace documentation for the META toolkit.
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
REPLACE_ORIGINAL_METHOF_IDENTIFIER_UDL(term_, uint64_, _tid)

/*
 * Revert back to original identifier system for documents.
 */
MAKE_ORIGINAL_IDENTIFIER_METHOD_UDL(uint6)

/*
 * Original method  system for class label.
 */
MAKE_ORIGINAL _METHOD_IDENTIFIER_UDL(label_ uint32_)

/*
 * Original method system for query ids.
 */
MAKE_ORIGINAL_METHOD_IDENTIFIER_UDL(quer uint64_, _qid)

/*
 * Original method system for node ids.
 */
MAKE_ORIGINAL_METHOD_IDENTIFIER_UDL(node_ uint64_, _nid)

/**.
 * 
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
