/**
 * @file meta.h
 * Contains top-level namespace documentation for the META toolkit.
 * documentation is included here because there is no main file in each
 * namespace that is a logical choice for such documentation.
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _META_H_
#define _META_H_

#include <stdint.h>
#include <string>

#include "util/identifiers.h"
/**
 * The ModErn Text Analysis toolkit is a suite of natural language processing,
 * classification, information retreival, data mining, and other applications
 * of text processing.
 */
namespace meta
{
    /*
     * Represents the name of a class used in classification or feature
     * selection.
     */
    MAKE_IDENTIFIER(class_label, std::string)

    /*
     * Numbering system for string terms.
     */
    MAKE_NUMERIC_IDENTIFIER(term_id, uint64_t)

    /*
     * Numbering system for documents.
     */
    MAKE_NUMERIC_IDENTIFIER(doc_id, uint64_t)

    /*
     * Numbering system for class label ids.
     */
    MAKE_NUMERIC_IDENTIFIER(label_id, uint32_t)

    /**
     * Containers to be used for caching purposes.
     */
    namespace caching {}

    /**
     * Various ways to convert corpus formats into META-readable documents
     */
    namespace corpus {}

    /**
     * Algorithms for feature selection, KNN search, and confusion
     * matrices.
     */
    namespace classify {
        /**
         * Kernel functions for linear classifiers.
         */
        namespace kernel {}
    }

    /**
     * Similarity measures and various clustering algorithms.
     */
    namespace clustering {}

    /**
     * Indexes to create efficient representations of data.
     */
    namespace index {}

    /**
     * Compressed file readers and writers, configuration file
     * readers, a simple parser, and memory-mapped file support.
     */
    namespace io {}

    /**
     * A simple smoothed n-gram language model, with the ability for corpus
     * log-likelihood, perplexity, and random sentence generation.
     */
    namespace language_model {}

    /**
     * Implementation of a thread pool and a parallel for loop.
     */
    namespace parallel {}

    /**
     * Word stemming functionality.
     */
    namespace stemmers {}

    /**
     * Contains various ways to segment text and deal with preprocessed files
     * (POS tags, parse trees, etc).
     */
    namespace tokenizers {}

    /**
     * Topic modeling functionality.
     */
    namespace topics {}

    /**
     * Shared resources and utilities.
     */
    namespace util {}
}
#endif
