#include <iostream>
#include "index/document.h"
#include "tokenizers/ngram_tokenizer.h"

using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
    Document doc("train.txt");
    NgramTokenizer tokenizer(1,
                             NgramTokenizer::Word,
                             NgramTokenizer::NoStemmer,
                             NgramTokenizer::NoStopwords);
    tokenizer.tokenize(doc);

    for(auto id: doc.getFrequencies())
        cout << tokenizer.getLabel(id.first) << " " << id.second << endl;

    return 0;
}
