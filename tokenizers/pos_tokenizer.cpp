/**
 * @file pos_tokenizer.cpp
 */

#include "index/document.h"
#include "parse_tree.h"
#include "pos_tokenizer.h"

using std::unordered_map;

void POSTokenizer::tokenize(Document & document,
        std::shared_ptr<unordered_map<TermID, unsigned int>> docFreq)
{

}
