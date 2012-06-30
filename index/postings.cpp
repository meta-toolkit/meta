/**
 * @file postings.cpp
 */

#include "postings.h"

Postings::Postings(const string & postingsFile):
    _reader(postingsFile),
    _postingsFilename(postingsFile)
{
    // ??
}

vector<PostingData> Postings::getDocs(const TermData & termData) const
{
    string line = getLine(termData.postingIndex);

    istringstream iss(line);
    vector<string> items;
    copy(std::istream_iterator<string>(iss),
         std::istream_iterator<string>(),
         std::back_inserter<vector<string>>(items));

    vector<PostingData> data;
    for(size_t i = 0; i < items.size(); i += 2)
    {
        PostingData postingData;
        istringstream(items[i]) >> postingData.docID;
        istringstream(items[i + 1]) >> postingData.freq;
        data.push_back(postingData);
    }

    return data;
}

vector<PostingData> Postings::getCompressedDocs(const TermData & termData) const
{
    vector<PostingData> data;
    return data;
}

string Postings::getLine(unsigned int lineNumber) const
{
    size_t currentLine = 0;
    ifstream infile(_postingsFilename, ifstream::in);
    string line;
    while(infile.good() && currentLine < lineNumber)
        std::getline(infile, line);
    infile.close();
    return line;
}

vector<string> Postings::createChunks(const vector<Document> & documents, size_t chunkMBSize, Tokenizer* tokenizer) const
{
    cerr << "[Postings]: creating chunks" << endl;

    vector<string> chunkNames;
    return chunkNames;
}

void Postings::createPostingsFile(const vector<string> & chunks)
{
    cerr << "[Postings]: merging chunks to create postings file" << endl;
}
