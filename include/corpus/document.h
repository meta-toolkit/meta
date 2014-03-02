/**
 * @file document.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_

#include <string>
#include <unordered_map>

#include "meta.h"
#include "util/optional.h"

namespace meta {

namespace corpus {

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
         */
        document(const std::string & path, doc_id d_id,
                 const class_label & label = class_label{"[NONE]"});

        /**
         * Increment the count of the specified transition.
         * @param term The string token whose count to increment
         * @param amount The amount to increment by
         */
        void increment(const std::string & term, double amount);

        /**
         * @return the path to this document (the argument to the constructor)
         */
        std::string path() const;

        /**
         * @return the classification category this document is in
         */
        const class_label & label() const;

        /**
         * @return the name of this document
         */
        std::string name() const;

        /**
         * @return the total of transitions recorded for this document.
         * This is not the number of unique transitions.
         */
        uint64_t length() const;

        /**
         * Get the number of occurrences for a particular term.
         * @param term The string term to look up
         */
        double count(const std::string & term) const;

        /**
         * @return the map of counts for this document.
         */
        const std::unordered_map<std::string, double> & counts() const;

        /**
         * Wrapper function for a document's cosine similarity measure.
         * @param a
         * @param b
         * @return the Jaccard similarity between the two parameters
         */
        static double jaccard_similarity(const document & a,
                                         const document & b);

        /**
         * Wrapper function for a document's cosine similarity measure.
         * @param a
         * @param b
         * @return the cosine similarity between the two parameters
         */
        static double cosine_similarity(const document & a, const document & b);

        /**
         * Sets the content of the document to be the parameter
         * @param content
         * @param encoding the encoding of content, which defaults to utf-8
         * @note saving the document's content is only used by some corpora
         * formats; not all documents are guaranteed to have content stored in
         * the object itself
         */
        void set_content(const std::string& content,
                         const std::string& encoding = "utf-8");

        /**
         * Sets the encoding for the document to be the parameter
         * @param encoding
         */
        void set_encoding(const std::string& encoding);

        /**
         * @return the contents of this document
         */
        const std::string & content() const;

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
         * @param label The label for this document
         */
        void set_label(class_label label);

    private:
        /** where this document is on disk */
        std::string _path;

        /** the document id for this document */
        doc_id _d_id;

        /** which category this document would be classified into */
        class_label _label;

        /** the short name for this document (not the full path) */
        std::string _name;

        /** the number of (non-unique) tokens in this document */
        size_t _length;

        /** counts of how many times each token appears */
        std::unordered_map<std::string, double> _counts;

        /** what the document contains */
        util::optional<std::string> _content;

        /** the encoding for the content */
        std::string _encoding;
};

}
}

#endif
