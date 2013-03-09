/**
 * @file tester.cpp
 */

#include <iostream>
#include "test/compressed_file_test.h"
#include "test/porter2_stemmer_test.h"
#include "test/parse_tree_test.h"
#include "test/unit_test.h"

using std::cerr;
using std::cout;
using std::endl;
using namespace UnitTests;

void porter2Stemmer()
{
    using namespace Tests::Porter2StemmerTests;

    cout << Common::makeBold("[porter2_stemmer]") << endl;
    runTest("stem", correctStem, 5);
    runTest("emptyStem", emptyStem);
    runTest("trim", trim);
}

void parseTree()
{
    using namespace Tests::ParseTreeTests;

    cout << Common::makeBold("[parse_tree]") << endl;
    runTest("constructor", constructor);
    runTest("height", height);
    runTest("getPOS", getPOS);
    runTest("getChildren", getChildren);
    runTest("numChildren", numChildren);
    runTest("getChildrenString", getChildrenString);
    runTest("getTrees", getTrees);
}

void compressedFile()
{
    using namespace Tests::CompressedFileTests;

    cout << Common::makeBold("[compressed_file]") << endl;
    runTest("init", init);
    runTest("write", testWrite);
    runTest("read", testRead);
    runTest("correct", correct);
    runTest("isSmaller", isSmaller);
}

int main(int argc, char* argv[])
{
    porter2Stemmer();
    parseTree();
    compressedFile();

    return 0;
}
