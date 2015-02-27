/**
 * @file analyzer.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_ANALYZER_H_
#define META_ANALYZER_H_

#include <stdexcept>
#include <memory>

#include "io/parser.h"

namespace cpptoml
{
class table;
}

namespace meta
{

namespace corpus
{
class document;
}

namespace analyzers
{

class token_stream;

/**
 * An class that provides a framework to produce token counts from documents.
 * All analyzers inherit from this class and (possibly) implement tokenize().
 */
class analyzer
{
  public:
    /**
     * A default virtual destructor.
     */
    virtual ~analyzer() = default;

    /**
     * Tokenizes a document.
     * @param doc The document to store the tokenized information in
     */
    virtual void tokenize(corpus::document& doc) = 0;

    /**
     * Clones this analyzer.
     */
    virtual std::unique_ptr<analyzer> clone() const = 0;

    /**
     * @param config The config group used to create the analyzer from
     * @return an analyzer as specified by a config object
     */
    static std::unique_ptr<analyzer> load(const cpptoml::table& config);

    /**
     * @param config The config group used to create the analyzer from
     * @return the default filter chain for this version of MeTA,
     * based on a config object
     */
    static std::unique_ptr<token_stream>
        default_filter_chain(const cpptoml::table& config);

    /**
     * @param global The original config object with all parameters
     * @param config The config group used to create the filters from
     * @return a filter chain as specified by a config object
     */
    static std::unique_ptr<token_stream>
        load_filters(const cpptoml::table& global,
                     const cpptoml::table& config);

    /**
     * @param src The token stream that will feed into this filter
     * @param config The config group used to create the filter from
     * @return a single filter specified by a config object
     */
    static std::unique_ptr<token_stream>
        load_filter(std::unique_ptr<token_stream> src,
                    const cpptoml::table& config);

    /**
     * @param doc The document to parse
     * @param extension The possible file extension for this document if it
     * is represented by a file on disk
     * @param delims Possible character delimiters to use when parsing the
     * file
     * @return a parser suited to read data that this document represents
     */
    static io::parser create_parser(const corpus::document& doc,
                                    const std::string& extension,
                                    const std::string& delims);

    /**
     * @param doc The document to get content for
     * @return the contents of the document, as a string
     */
    static std::string get_content(const corpus::document& doc);

  public:
    /**
     * Basic exception for analyzer interactions.
     */
    class analyzer_exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };
};
}
}
#endif
