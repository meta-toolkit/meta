/**
 * @file search.h
 */

#ifndef _SEARCH_H_
#define _SEARCH_H_

#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include "parse_tree.h"
#include "document.h"

using std::ifstream;
using std::vector;
using std::string;

/**
 *
 */
namespace search
{
    /**
     *
     */
    void readIndex(string indexPath);

    /**
     * Scores a document given a query.
     * @param document - the doc to score
     * @param query - the query to score against
     * @return the real score value 
     */
    double scoreDocument(const Document & document, const Document & query);
}

#endif
