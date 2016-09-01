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
#include <vector>

#include "meta/config.h"
#include "meta/corpus/metadata.h"
#include "meta/meta.h"
#include "meta/util/optional.h"

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
     * @param d_id The doc id to assign to this document
     * @param label The optional class label to assign this document
     */
    document(doc_id d_id = doc_id{0},
             const class_label& label = class_label{"[NONE]"});

    /**
     * @return the classification category this document is in
     */
    const class_label& label() const;

    /**
     * Sets the content of the document to be the parameter
     * @param content The string content to assign into this document
     * @param encoding the encoding of content, which defaults to utf-8
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

    /**
     * @return the set of extra metadata fields for this document
     */
    const std::vector<metadata::field>& mdata() const;

    /**
     * Sets the extra metadata fields for this document
     * @param metadata The new metadata for this document
     */
    void mdata(std::vector<metadata::field>&& metadata);

  private:
    /// The document id for this document
    doc_id d_id_;

    /// Which category this document would be classified into
    class_label label_;

    /// Other metadata fields for this document
    std::vector<metadata::field> mdata_;

    /// What the document contains
    util::optional<std::string> content_;

    /// The encoding for the content
    std::string encoding_;
};
}
}

#endif
