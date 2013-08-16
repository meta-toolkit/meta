/**
 * @file meta.h
 * Contains top-level namespace documentation for the META toolkit.
 * documentation is included here because there is no main file in each
 * namespace that is a logical choice for such documentation.
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#include <stdint.h>
#include <string>

/**
 * The ModErn Text Analysis toolkit is a suite of natural language processing,
 * classification, information retreival, data mining, and other applications
 * of text processing.
 */
namespace meta
{
    /**
     * Represents the name of a class used in classification or feature
     * selection.
     */
    typedef std::string class_label;

    /**
     * Numbering system for string terms.
     */
    typedef uint64_t term_id;

    /**
     * Numbering system for documents.
     */
    typedef uint64_t doc_id;

    /**
     * Algorithms for feature selection, KNN search, and confusion
     * matrices.
     */
    namespace classify {}

    /**
     * Similarity measures and various clustering algorithms.
     */
    namespace clustering {}

    /**
     * Two types of indices (RAMIndex, InvertedIndex) and associated
     * classes.
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
     * Topic modeling functionality 
     */
    namespace topics {}

    /**
     * Shared resources and 
     */
    namespace util {}
}
