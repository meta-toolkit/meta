/**
 * @file lm.cpp
 */

#include <iostream>
#include "model/ngram_distribution.h"

int main(int argc, char* argv[])
{
    NgramDistribution<4> dist("train.txt");

    return 0;
}
