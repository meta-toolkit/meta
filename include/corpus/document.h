/**
 * @file document.h
 * @author Sean Massung
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_DOCUMENT_H_
#define META_DOCUMENT_H_

#include <string>
#include <unordered_map>

#include "meta.h"
#include "util/optional.h"

namespace meta
{
namespace corpus
{

/**
 * Represents an indexable document. Internally, a document may contain either
 * string content or a path to a file it represents on disk.
 *
 * Once tokenized, a document contains a mapping of term -> frequency. This
 * mapping is empty upon creation.
 */
class document
{
  public:
    /**
     * Constructor.
     * @param path The path to the document
     * @param d_id The doc id to assign to this document
     * @param label The optional class label to assign this document
     */
    document(const std::string& path = "[NONE]", doc_id d_id = doc_id{0},
             const class_label& label = class_label{"[NONE]"});

    /**
     * Increment the count of the specified transition.
     * @param term The string token whose count to increment
     * @param amount The amount to increment by
     */
    void increment(const std::string& term, double amount);

    /**
     * @return the path to this document (the argument to the constructor)
     */
    std::string path() const;

    /**
     * @return the classification category this document is in
     */
    const class_label& label() const;

    /**
     * @return the name of this document
     */
    std::string name() const;

    /**
     * @param n The new name for this document
     */
    void name(const std::string& n);

    /**
     * @return the total of transitions recorded for this document.
     * This is not the number of unique transitions.
     */
    uint64_t length() const;

    /**
     * Get the number of occurrences for a particular term.
     * @param term The string term to look up
     * @return the number of times term appears in this document
     */
    double count(const std::string& term) const;

    /**
     * @return the map of counts for this document.
     */
    const std::unordered_map<std::string, double>& counts() const;

    /**
     * Sets the content of the document to be the parameter
     * @param content The string content to assign into this document
     * @param encoding the encoding of content, which defaults to utf-8
     * @note saving the document's content is only used by some corpora
     * formats; not all documents are guaranteed to have content stored in
     * the object itself
     */
    void content(const std::string& content,
                 const std::string& encoding = "utf-8");

    /**
     * Sets the encoding for the document to be the parameter
     * @param encoding The string label for the encoding
     */
    void encoding(const std::string& encoding);

    /**
     * @return the contents of this document
     */
    const std::string& content() const;

    /**
     * @return the encoding for this document
     */
    const std::string& encoding() const;

    /**
     * @return the doc_id for this document
     */
    doc_id id() const;

    /**
     * @return whether this document contains its content internally
     */
    bool contains_content() const;

    /**
     * Sets the label for this document.
     * @param label The new label for this document
     */
    void label(class_label label);

  private:
    /// Where this document is on disk
    std::string path_;

    /// The document id for this document
    doc_id d_id_;

    /// Which category this document would be classified into
    class_label label_;

    /// The short name for this document (not the full path)
    std::string name_;

    /// The number of (non-unique) tokens in this document
    size_t length_;

    /// Counts of how many times each token appears
    std::unordered_map<std::string, double> counts_;

    /// What the document contains
    util::optional<std::string> content_;

    /// The encoding for the content
    std::string encoding_;
};
}
}

#endif
