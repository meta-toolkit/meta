/**
 * @file postings.cpp
 */

#include "postings.h"

Postings::Postings(const string & postingsFile):
    _reader(postingsFile),
    _postingsFilename(postingsFile),
    _docMap(InvertibleMap<DocID, string>()),
    _currentDocID(0)
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

size_t Postings::createChunks(vector<Document> & documents, size_t chunkMBSize, Tokenizer* tokenizer)
{
    cerr << "[Postings]: creating chunks" << endl;

    size_t chunkNum = 0;
    vector<string> chunkNames;
    map<TermID, vector<PostingData>> terms;

    /*
        Create Map of TermID to vector of PostingData.
          - TermIDs need to be sorted on disk for merge.
          - PostingData needs to be sorted when it's all there (call std::sort on vector)
             so don't worry about that here; it'll happen in createPostingsFile()
          - When map.size() * (sizeof(TermID) + sizeof(PostingData)) >= (chunkMBSize * 1024 * 1024),
             write current map to disk
    */

    // iterate over documents, writing data to disk when we reach chunkMBSize
    for(auto & doc: documents)
    {
        tokenizer->tokenize(doc, NULL);
        unordered_map<TermID, unsigned int> freqs = doc.getFrequencies();
        for(auto & freq: freqs)
        {
            DocID docID = getDocID(doc.getPath());
            terms[freq.first].push_back(PostingData(docID, freq.second));
        }

        // check size of term map
        //if(terms.size() * (sizeof(TermID) + sizeof(PostingData)) >= (chunkMBSize * 1024 * 1024))
        if(terms.size() * (sizeof(TermID) + sizeof(PostingData)) >= (chunkMBSize * 1024))
            writeChunk(terms, chunkNum++);
    }

    // write the last partial chunk
    writeChunk(terms, chunkNum++);

    return chunkNum;
}

DocID Postings::getDocID(const string & path)
{
    if(_docMap.containsValue(path))
        return _docMap.getKeyByValue(path);
    else
    {
        _docMap.insert(_currentDocID, path);
        return _currentDocID++;
    }
}

void Postings::writeChunk(map<TermID, vector<PostingData>> & terms, size_t chunkNum) const
{
    std::stringstream ss;
    ss << chunkNum;
    string fileNumber = ss.str();
    ofstream outfile(fileNumber + ".chunk");

    if(outfile.good())
    {
        for(auto & term: terms)
        {
            outfile << term.first;
            for(auto & pdata: term.second)
                outfile << " " << pdata.docID << " " << pdata.freq;
            outfile << "\n";
        }

        outfile.close();
        terms.clear();
    }
    else
    {
        cerr << "[Postings]: error creating chunk file" << endl;
    }
}

void Postings::createPostingsFile(size_t numChunks)
{
    cerr << "[Postings]: merging chunks to create postings file" << endl;
    // the lexicon can be created when the postings file is written to disk

    ofstream postingsFile(_postingsFilename);
    if(!postingsFile.good())
    {
        cerr << "[Postings]: error creating postings file" << endl;
        return;
    }

    postingsFile.close();
}
