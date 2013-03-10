/**
 * @file postings.cpp
 */

#include "tokenizers/tokenizer.h"
#include "io/compressed_file_reader.h"
#include "io/compressed_file_writer.h"
#include "index/lexicon.h"
#include "index/structs.h"
#include "index/chunk_list.h"
#include "index/postings.h"

using std::map;
using std::istringstream;
using std::ifstream;
using std::string;
using std::vector;
using std::cerr;
using std::endl;

Postings::Postings(const string & postingsFile):
    //_reader(postingsFile),
    _postingsFilename(postingsFile),
    _docMap(InvertibleMap<DocID, string>()),
    _currentDocID(0)
{ /* nothing */ }

vector<PostingData> Postings::getDocs(const TermData & termData) const
{
    string line = getLine(termData.postingIndex);

    istringstream iss(line);
    vector<string> items;
    copy(std::istream_iterator<string>(iss),
         std::istream_iterator<string>(),
         std::back_inserter<vector<string>>(items));

    vector<PostingData> data;
    for(size_t i = 1; i < items.size(); i += 2)
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
    while(infile.good() && currentLine++ < lineNumber + 1)
        std::getline(infile, line);
    infile.close();
    return line;
}

size_t Postings::createChunks(vector<Document> & documents, size_t chunkMBSize,
        std::shared_ptr<Tokenizer> tokenizer)
{
    cerr << "[Postings]: creating chunks" << endl;

    size_t chunkNum = 0;
    size_t maxSize = chunkMBSize * 4 * 1024 * 1024;
    vector<string> chunkNames;
    map<TermID, vector<PostingData>> terms;

    // iterate over documents, writing data to disk when we reach chunkMBSize
    for(auto & doc: documents)
    {
        tokenizer->tokenize(doc);
        DocID docID = getDocID(doc.getPath());
        cerr << " -> tokenizing " << doc.getName() << " (docid " << docID << ")" << endl;
        unordered_map<TermID, unsigned int> freqs = doc.getFrequencies();
        for(auto & freq: freqs)
            terms[freq.first].push_back(PostingData(docID, freq.second));

        if(terms.size() * (sizeof(TermID) + sizeof(PostingData)) >= maxSize)
            writeChunk(terms, chunkNum++);
    }

    // write the last partial chunk
    if(terms.size())
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

void Postings::createPostingsFile(size_t numChunks, Lexicon & lexicon)
{
    cerr << "[Postings]: merging chunks to create postings file" << endl;
    ofstream postingsFile(_postingsFilename);
    if(!postingsFile.good())
    {
        cerr << "[Postings]: error creating postings file" << endl;
        return;
    }

    size_t line = 0;
    ChunkList chunks(numChunks);
    while(chunks.hasNext())
    {
        IndexEntry entry = chunks.next();

        TermData termData;
        termData.idf = entry.data.size();
        termData.totalFreq = getTotalFreq(entry.data);
        termData.postingIndex = line++;
        termData.postingBit = 0; // uncompressed, always 0

        lexicon.addTerm(entry.termID, termData);
        postingsFile << entry.toString() << "\n";
    }

    postingsFile.close();
}

void Postings::saveDocIDMapping(const string & filename) const
{
    _docMap.saveMap(filename);
}

unsigned int Postings::getTotalFreq(const vector<PostingData> & pdata) const
{
    unsigned int freq = 0;
    for(auto & d: pdata)
        freq += d.freq;
    return freq;
}

void Postings::saveDocLengths(const vector<Document> & documents, const string & filename)
{
    ofstream outfile(filename);
    if(outfile.good())
    {
        for(auto & doc: documents)
            outfile << getDocID(doc.getPath()) << " " << doc.getLength() << endl;
    }
    else
    {
        cerr << "[Postings]: error saving document lengths" << endl;
    }
}
