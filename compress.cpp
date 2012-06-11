#include <utility>
#include <map>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <omp.h>

#include "textfile.h"
#include "invertible_map.h"
#include "compressed_file_reader.h"
#include "compressed_file_writer.h"

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
    for(unordered_map<char, size_t>::const_iterator it = freqs.begin(); it != freqs.end(); ++it)
        sorted.insert(make_pair(it->second, it->first));

    InvertibleMap<char, unsigned int> mapping;
    unsigned int value = 1;
    for(multimap<size_t, char>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it, ++value)
        mapping.insert(it->second, value);

    return mapping;
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "usage: " << argv[0] << " file.txt" << endl;
        return 1;
    }

    cerr << "Getting frequencies..." << endl;
    double start = omp_get_wtime();
    unordered_map<char, size_t> freqs = getFreqs(string(argv[1]));
    cerr << "  found " << freqs.size() << " unique characters" << endl;
    cerr << "  " << omp_get_wtime() - start << " seconds elapsed" << endl;

    InvertibleMap<char, unsigned int> mapping = getMapping(freqs);

    cerr << "Compressing..." << endl;
    start = omp_get_wtime();
    compress(string(argv[1]), "compressed.txt", mapping);
    cerr << "  " << omp_get_wtime() - start << " seconds elapsed" << endl;

    cerr << "Decompressing..." << endl;
    start = omp_get_wtime();
    decompress("compressed.txt", "uncompressed.txt", mapping);
    cerr << "  " << omp_get_wtime() - start << " seconds elapsed" << endl;

    return 0;
}
