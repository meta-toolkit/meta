/**
 * @file lm.cpp
 */

#include <iostream>
#include "model/ngram_distribution.h"

using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        cout << "Usage: " << argv[0] << " seed numWords" << endl;
        return 1;
    }

    meta::language_model::NgramDistribution<4> dist("papers.txt");
    cout << endl << dist.random_sentence(atoi(argv[1]), atoi(argv[2])) << endl << endl;

    return 0;
}
