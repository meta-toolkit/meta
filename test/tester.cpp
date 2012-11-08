/**
 * @file tester.cpp
 */

#include <iostream>
#include <omp.h>
#include "test/compressed_file_test.h"
#include "test/porter2_stemmer_test.h"
#include "test/parse_tree_test.h"
#include "test/unit_test.h"

using std::cerr;
using std::cout;
using std::endl;
using namespace UnitTests;

/*
unordered_map<char, size_t> getFreqs(string filename)
{
    unordered_map<char, size_t> freqs;
    TextFile textfile(filename);

    char* start = textfile.opentext();
    unsigned int length = textfile.get_size();
    unsigned int index = 0;

    while(index < length)
    {
        ++freqs[start[index]];
        if(++index % 1000 == 0)
            cerr << "  " << (double) index / length * 100 << "%    \r";
    }
    cerr << "  100.00%  " << endl;

    textfile.closetext();
    return freqs;
}

void compress(string filename, string outfilename, const InvertibleMap<char, unsigned int> & mapping)
{
    TextFile textfile(filename);
    CompressedFileWriter writer(outfilename);

    char* start = textfile.opentext();
    unsigned int length = textfile.get_size();
    unsigned int index = 0;

    while(index < length)
    {
        unsigned int toWrite = mapping.getValueByKey(start[index]);
        writer.write(toWrite);
        if(++index % 1000 == 0)
            cerr << "  " << (double) index / length * 100 << "%    \r";
    }
    cerr << "  100.00%  " << endl;

    textfile.closetext();
}

void decompress(string infilename, string outfilename, const InvertibleMap<char, unsigned int> & mapping)
{
    CompressedFileReader reader(infilename);
    ofstream writer(outfilename);
    unsigned int val;
    while(reader.hasNext())
    {
        val = reader.next();
        writer << mapping.getKeyByValue(val);
    }
    writer.close();
}

InvertibleMap<char, unsigned int> getMapping(const unordered_map<char, size_t> & freqs)
{
    multimap<size_t, char> sorted;
    for(auto & it: freqs)
        sorted.insert(make_pair(it.second, it.first));

    InvertibleMap<char, unsigned int> mapping;
    unsigned int value = 1;
    for(auto it = sorted.rbegin(); it != sorted.rend(); ++it)
    {
        mapping.insert(it->second, value);
        ++value;
    }

    return mapping;
}

void testCompression(string filename)
{
    cerr << "Getting frequencies..." << endl;
    double start = omp_get_wtime();
    unordered_map<char, size_t> freqs = getFreqs(string(filename));
    cerr << "  found " << freqs.size() << " unique characters" << endl;
    cerr << "  " << omp_get_wtime() - start << " seconds elapsed" << endl;

    InvertibleMap<char, unsigned int> mapping = getMapping(freqs);

    cerr << "Compressing..." << endl;
    start = omp_get_wtime();
    compress(string(filename), "compressed.txt", mapping);
    cerr << "  " << omp_get_wtime() - start << " seconds elapsed" << endl;

    cerr << "Decompressing..." << endl;
    start = omp_get_wtime();
    decompress("compressed.txt", "uncompressed.txt", mapping);
    cerr << "  " << omp_get_wtime() - start << " seconds elapsed" << endl;
}
*/

void thing()
{
    ASSERT(1 == 1);
    ASSERT(2 == 3);
    PASS;
}

void thing1()
{
    int* p = NULL;
    *p = 1;
    PASS;
}

void thing2()
{
    while(1);
    PASS;
}

void thing3()
{
    PASS;
}

void testTest()
{
    cout << "Running tests..." << endl;
    UnitTests::runTest("MyFirstTest", thing);
    UnitTests::runTest("MySecondTest", thing1);
    UnitTests::runTest("InfiniteLoopTest", thing2, 3);
    UnitTests::runTest("LastTest", thing3);
}

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
    runTest("read", testRead);
    runTest("write", testWrite);
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
