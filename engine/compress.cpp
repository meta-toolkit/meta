#include "compressed_file_reader.h"
#include "compressed_file_writer.h"

#include <utility>
#include <map>
#include <unordered_map>
#include <fstream>
#include <iostream>

using std::make_pair;
using std::pair;
using std::multimap;
using std::unordered_map;
using std::ifstream;
using std::cerr;
using std::cout;
using std::endl;

unordered_map<char, size_t> getFreqs(string filename)
{
    unordered_map<char, size_t> freqs;
    ifstream infile(filename, ifstream::in);
    if(infile.is_open())
    {
        string line;
        while(infile.good())
        {
            std::getline(infile, line);
            for(size_t i = 0; i < line.size(); ++i)
                ++freqs[line[i]];
        }        
        infile.close();
    }
    else
    {
        cerr << "Failed to open " << filename << endl;
    }
    return freqs;
}

// yes, I know u_m is overkill for chars, but this will be general
void compress(string filename, const unordered_map<char, unsigned int> & mapping)
{
    ifstream infile(filename, ifstream::in);
    if(infile.is_open())
    {
        CompressedFileWriter writer(filename + ".compressed");
        string line;
        while(infile.good())
        {
            std::getline(infile, line);
            for(size_t i = 0; i < line.size(); ++i)
                writer.write(mapping.at(line[i]));
        }        
        infile.close();
    }
    else
    {
        cerr << "Failed to open " << filename << endl;
    }

}

void decompress(string filename, const unordered_map<unsigned int, char> & reverseMapping)
{
    CompressedFileReader reader(filename);
    while(reader.hasNext())
    {
        unsigned int val = reader.next();
        unordered_map<unsigned int, char>::const_iterator it = reverseMapping.find(val);
        if(it == reverseMapping.end())
            cerr << "uhoh" << endl;
        else
            cout << it->second;
    }
}

unordered_map<char, unsigned int> getMapping(const unordered_map<char, size_t> & freqs)
{
    multimap<size_t, char> sorted;

    for(unordered_map<char, size_t>::const_iterator it = freqs.begin(); it != freqs.end(); ++it)
        sorted.insert(make_pair(it->second, it->first));

    unordered_map<char, unsigned int> mapping;

    unsigned int value = 1;
    for(multimap<size_t, char>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it, ++value)
        mapping.insert(make_pair(it->second, value));
    return mapping;
}

unordered_map<unsigned int, char> getReverseMapping(unordered_map<char, unsigned int> mapping)
{
    unordered_map<unsigned int, char> rmap;
    for(unordered_map<char, unsigned int>::iterator it = mapping.begin(); it != mapping.end(); ++it)
        rmap.insert(make_pair(it->second, it->first));
    return rmap;
}

int main(int argc, char* argv[])
{
    cerr << "get freqs\n";
    unordered_map<char, size_t> freqs = getFreqs("aliceinwonderland.txt");
    cerr << "  freqs size: " << freqs.size() << endl;

    cerr << "get mapping\n";
    unordered_map<char, unsigned int> mapping = getMapping(freqs);
    cerr << "  mapping size: " << mapping.size() << endl;

    cerr << "compress\n";
    compress("aliceinwonderland.txt", mapping);

    cerr << "get reverse mapping\n";
    unordered_map<unsigned int, char> reverseMapping = getReverseMapping(mapping);
    cerr << "  reverse mapping size: " << reverseMapping.size() << endl;

    cerr << "decompress\n";
    decompress("aliceinwonderland.txt.compressed", reverseMapping);

    return 0;
}
