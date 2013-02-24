#include <iostream>
#include "model/ngram_distribution.h"

int main(int argc, char* argv[])
{
    NgramDistribution<2> dist("train.txt");

    return 0;
}
