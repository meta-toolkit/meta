/**
 * @file tester.cpp
 */

#include <vector>
#include <utility>
#include <map>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <omp.h>

#include "io/textfile.h"
#include "io/compressed_file_reader.h"
#include "io/compressed_file_writer.h"
#include "index/lexicon.h"
#include "index/inverted_index.h"
#include "tokenizers/ngram_tokenizer.h"
#include "util/invertible_map.h"

using std::vector;
using std::make_pair;
using std::pair;
using std::multimap;
using std::unordered_map;
using std::ofstream;
using std::cerr;
using std::cout;
using std::endl;

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
    for(auto & it: sorted)
    {
        mapping.insert(it.second, value);
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

void testLexicon()
{
    Lexicon lexicon("lexicon.txt");
    TermData data = lexicon.getTermInfo(1);
    cerr << "idf for 1: " << data.idf << endl;
    data.idf = 77;
    data.totalFreq = 77;
    data.postingIndex = 77;
    data.postingBit = 7;
    lexicon.addTerm(77, data);
    lexicon.save();
}

void testIterators()
{
    InvertibleMap<int, string> imap;
    imap.insert(4, "derp");
    imap.insert(5, "derpyderp");
    imap.insert(7, "derpderp");
    imap.insert(19, "herpderp");
    for(InvertibleMap<int, string>::const_iterator it = imap.begin(); it != imap.end(); ++it)
        cout << it->first << " " << it->second << endl;
    for(auto & it: imap)
        cout << it.first << " " << it.second << endl;
}

vector<Document> getDocs(const string & filename, const string & prefix)
{
    vector<Document> docs;
    Parser parser(filename, "\n");
    while(parser.hasNext())
    {
        string file = parser.next();
        docs.push_back(Document(prefix + file));
    }
    return docs;
}

void testIndex()
{
    string prefix = "/home/sean/projects/senior-thesis-data/6reviewers/";
    string lexicon = "lexiconFile";
    string postings = "postingsFile";
    vector<Document> trainDocs = getDocs(prefix + "train.txt", prefix);
    Tokenizer* tokenizer = new NgramTokenizer(1);
    InvertedIndex index(lexicon, postings, tokenizer);
    index.indexDocs(trainDocs, 1);
}

int main(int argc, char* argv[])
{
    /*
    if(argc != 2)
    {
        cerr << "usage: " << argv[0] << " file.txt" << endl;
        return 1;
    }
    */

    //testCompression(string(argv[0]));
    //testLexicon();
    //testIterators();
    testIndex();

    return 0;
}
