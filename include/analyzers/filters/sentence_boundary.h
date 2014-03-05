/**
 * @file sentence_boundary.h
 * @author Chase Geigle
 */

#ifndef _META_SENTENCE_BOUNDARY_H_
#define _META_SENTENCE_BOUNDARY_H_

#include <deque>
#include <memory>
#include <unordered_set>

#include "analyzers/filter_factory.h"
#include "util/clonable.h"
#include "util/optional.h"

namespace cpptoml
{
class toml_group;
}

namespace meta
{
namespace analyzers
{

/**
 * Filter that adds sentence boundary tokens ("<s>" and "</s>") to streams of
 * tokens. This filter requires that whitespace and punctuation be present
 * in the source stream.
 */
class sentence_boundary : public util::clonable<token_stream, sentence_boundary>
{
  public:
    /**
     * Loads the maps that contain the heuristics for the sentence
     * boundary instances.
     */
    static void load_heuristics(const cpptoml::toml_group& config);

    /**
     * Constructs a sentence_boundary filter, reading tokens from the
     * given source and configured via the given configuration group.
     */
    sentence_boundary(std::unique_ptr<token_stream> source);

    /**
     * Copy constructor.
     */
    sentence_boundary(const sentence_boundary& other);

    /**
     * Sets the content for the beginning of the filter chain.
     */
    void set_content(const std::string& content) override;

    /**
     * Obtains the next token in the sequence.
     */
    std::string next() override;

    /**
     * Determines whether there are more tokens available in the stream.
     */
    operator bool() const override;

    /**
     * Identifier for this filter.
     */
    const static std::string id;

  private:
    /**
     * Returns the next buffered token.
     */
    std::string current_token();

    /**
     * Determines if the given token is a possible end of sentence
     * punctuation marker.
     */
    static bool possible_punc(const std::string& token);

    /**
     * Determines if the given token can be the last word in a sentence.
     */
    static bool possible_end(const std::string& token);

    /**
     * Determines if the given token can be the beginning of a sentence.
     */
    static bool possible_start(const std::string& token);

    /**
     * The source to read tokens from.
     */
    std::unique_ptr<token_stream> source_;

    /**
     * The current buffered tokens.
     */
    std::deque<std::string> tokens_;

    /**
     * The previous token.
     */
    util::optional<std::string> prev_;

    /**
     * The set of possible punctuation marks. Shared among all instances.
     */
    static std::unordered_set<std::string> punc_set;

    /**
     * The set of words that may not start sentences. Shared among all
     * instances.
     */
    static std::unordered_set<std::string> start_exception_set;

    /**
     * The set of words that may not end sentences. Shared among all
     * instances.
     */
    static std::unordered_set<std::string> end_exception_set;

    /**
     * Whether or not the heuristics above have been loaded. Must be set by
     * loading the heuristics before constructing any sentence_boundary
     * filters.
     */
    static bool heuristics_loaded;
};

/**
 * Specialization of the factory method used to create sentence_boundary
 * filters.
 */
template <>
std::unique_ptr<token_stream>
    make_filter<sentence_boundary>(std::unique_ptr<token_stream>,
                                   const cpptoml::toml_group&);
}
}
#endif
