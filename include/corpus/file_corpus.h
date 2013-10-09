/**
 * @file file_corpus.h
 * @author Sean Massung
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#include <string>
#include <vector>
#include <utility>
#include "io/parser.h"
#include "corpus/corpus.h"

namespace meta {
namespace corpus {

/**
 * Creates document objects from individual files, each representing a single
 * document.
 */
class file_corpus: public corpus
{
    public:
        /**
         * @param prefix The path to where the files are located
         * @param doc_list A file containing the path to each document in the
         * corpus
         */
        file_corpus(const std::string & prefix, const std::string & doc_list);

        /**
         * @return whether there is another document in this corpus
         */
        bool has_next() const override;

        /**
         * @return the next document from this corpus
         */
        document next() override;

        /**
         * @return the number of documents in this corpus
         */
        uint64_t size() const override;

    private:
        /** the current document we are on */
        uint64_t _cur;

        /** the path to all the documents */
        std::string _prefix;

        /** contains doc class labels and paths */
        std::vector<std::pair<std::string, class_label>> _docs;
};

}
}
