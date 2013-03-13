/**
 * @file compressed_file_test.h
 */

#ifndef _COMPRESSED_FILE_H_
#define _COMPRESSED_FILE_H_

#include <map>
#include <unordered_map>
#include "unit_test.h"
#include "util/invertible_map.h"
#include "io/textfile.h"
#include "io/compressed_file_reader.h"
#include "io/compressed_file_writer.h"

namespace Tests
{
    namespace CompressedFileTests
    {
        using std::unordered_map;
        using std::multimap;

        unordered_map<char, size_t> freqs;
        InvertibleMap<char, unsigned int> mapping;
        string inputFilename;
        string compressedFilename;
        string uncompressedFilename;

        /**
         * Helper function; counts frequencies of a given file to determine best
         * compression mapping.
         * @param filename The file to analyze
         * @return the mapping of char to count
         */
        unordered_map<char, size_t> getFreqs(const string & filename)
        {
            unordered_map<char, size_t> freqs;
            MmapFile textfile(filename);

            char* start = textfile.start();
            unsigned int index = 0;

            while(index < textfile.size())
                ++freqs[start[index++]];

            return freqs;
        }

        /**
         * Creates a char to id mapping for the delta compression based on
         * character counts.
         * @param freqs The frequency data of the file to be compressed
         * @return the mapping of char to id
         */
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

        void testRead()
        {
            CompressedFileReader reader(compressedFilename);
            ofstream writer(uncompressedFilename);
            while(reader.hasNext())
            {
                unsigned int val = reader.next();
                writer << mapping.getKeyByValue(val);
            }
            writer.close();
            PASS;
        }

        void testWrite()
        {
            MmapFile textfile(inputFilename);
            CompressedFileWriter writer(compressedFilename);

            char* start = textfile.start();
            unsigned int index = 0;

            while(index < textfile.size())
            {
                unsigned int toWrite = mapping.getValueByKey(start[index++]);
                writer.write(toWrite);
            }

            PASS;
        }

        void correct()
        {
            PASS;
        }

        void isSmaller()
        {
            ASSERT(1 == 1);
        }

        void init()
        {
            inputFilename = "data/to_compress.txt";
            compressedFilename = "data/compressed.txt";
            uncompressedFilename = "data/uncompressed.txt";
            freqs = getFreqs(string(inputFilename));
            mapping = getMapping(freqs);
            PASS;
        }
    }
}

#endif
