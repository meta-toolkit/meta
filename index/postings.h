/**
 * @file postings.h
 */

#ifndef _POSTINGS_H_
#define _POSTINGS_H_

#include <sstream>
#include <fstream> // to remove
#include <string>
#include <vector>
#include <iostream>
#include "io/compressed_file_reader.h"
#include "io/compressed_file_writer.h"
#include "lexicon.h"

using std::istringstream;
using std::ifstream; // to remove
using std::string;
using std::vector;
using std::cerr;
using std::endl;

/**
 * Represents one term's document info.
 */
struct PostingData
{
    DocID docID;
    unsigned int freq;
};

/**
 * This is the interface to the large postings file located on disk.
 */
class Postings
{
    public:

        /**
         * Constructor; sets this Postings object to look at the specified file.
         */
        Postings(const string & postingsFile);

        /**
         * @param termData - used to determine the location of the term information in the postings
         * @return a vector of documents that contain the term the parameters refer to
         */
        vector<PostingData> getDocs(const TermData & termData) const;

        /**
         * @param termData - used to determine the location of the term information in the postings
         * @return a vector of documents that contain the term the parameters refer to
         */
        vector<PostingData> getCompressedDocs(const TermData & termData) const;

    private:

        string _postingsFilename;
        CompressedFileReader _reader;

        /**
         * Gets a line out of an uncompressed postings file.
         * This will be slow for large posting files and should
         *  only be used to check correctness.
         */
        string getLine(unsigned int lineNumber) const;
};

#endif
