/**
 * @file index.h
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _INDEX_H_
#define _INDEX_H_

#include <map>
#include <vector>
#include <exception>
#include <string>
#include "index/document.h"

namespace meta {
namespace index {

/**
 * An abstract class that represents searchable document-based index.
 */
class Index
{
    public:

        /**
         * Searches the index using the scoredocument function on each document.
         * @param query - the query to perform the search with
         * @return - a mapping of scores to documents
         */
        virtual std::multimap<double, std::string> search(document & query) const = 0;

        /**
         * Scores a document given a query.
         * @param document The doc to score
         * @param query The query to score against
         * @return the real score value 
         */
        virtual double score_doc(const document & doc, const document & query) const = 0;

        /**
         * Basic exception for Index interactions.
         */
        class index_exception: public std::exception
        {
            public:
                
                index_exception(const std::string & error):
                    _error(error) { /* nothing */ }

                const char* what () const throw ()
                {
                    return _error.c_str();
                }
           
            private:
           
                std::string _error;
        };
};

}
}

#endif
