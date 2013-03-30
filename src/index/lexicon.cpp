/**
 * @file lexicon.cpp
 */

#include "io/parser.h"
#include "util/common.h"
#include "index/document.h"
#include "index/lexicon.h"
#include "index/index.h"

namespace meta {
namespace index {

using std::ofstream;
using std::vector;
using std::istringstream;
using std::endl;
using std::cerr;
using std::pair;
using std::make_pair;
using std::unordered_map;
using std::string;

using util::InvertibleMap;
using io::Parser;

Lexicon::Lexicon(const string & lexiconFile):
    _lexiconFilename(lexiconFile),
    _entries(unordered_map<TermID, TermData>()),
    _docLengths(unordered_map<DocID, unsigned int>()),
    _termMap(InvertibleMap<TermID, string>()),
    _docMap(InvertibleMap<DocID, string>())
{
    readLexicon();
    setAvgDocLength();
}

bool Lexicon::isEmpty() const
{
    return _entries.empty();
}

TermData Lexicon::getTermInfo(TermID termID) const
{
    auto it = _entries.find(termID);
    if(it == _entries.end())
    {
        cerr << "[Lexicon]: warning: termID lookup failed" << endl;
        return TermData();
    }
    return it->second;
}

void Lexicon::save(const string & docLengthsFilename, const string & termMapFilename, const string & docMapFilename) const
{
    cerr << "[Lexicon]: saving lexicon to disk" << endl;

    ofstream outfile(_lexiconFilename);
    if(outfile.good())
    {
        outfile << docLengthsFilename << endl;
        outfile << termMapFilename << endl;
        outfile << docMapFilename << endl;

        for(auto & entry: _entries)
        {
            outfile << entry.first << " ";
            TermData data = entry.second;
            outfile << data.idf << " ";
            outfile << data.totalFreq << " ";
            outfile << data.postingIndex << " ";
            outfile << static_cast<int>(data.postingBit) << "\n";
        }
        outfile.close();
    }
    else
        throw Index::index_exception("[Lexicon]: error writing lexicon to disk");
}

void Lexicon::addTerm(TermID term, TermData termData)
{
    _entries.insert(make_pair(term, termData));
}

void Lexicon::readLexicon()
{
    Parser parser(_lexiconFilename, "\n");
    
    /*
    if(!parser.isValid())
    {
        cerr << "[Lexicon]: created empty lexicon" << endl;
        return;
    }
    */

    // the first, second, and third lines in the lexicon file correspond
    //  to the doclengths files, term id mapping, and docid mapping files respectively

    cerr << "[Lexicon]: reading doc lengths from file" << endl;
    setDocLengths(parser.next());
    cerr << " -> added " << _docLengths.size() << " document lengths" << endl;
    
    cerr << "[Lexicon]: reading termIDs from file" << endl;
    _termMap.readMap(parser.next());
    cerr << " -> added " << _termMap.size() << " terms" << endl;

    cerr << "[Lexicon]: reading docIDs from file" << endl;
    _docMap.readMap(parser.next());
    cerr << " -> added " << _docMap.size() << " documents" << endl;

    while(parser.hasNext())
    {
        istringstream line(parser.next());
        vector<string> items;

        std::copy(std::istream_iterator<string>(line),
             std::istream_iterator<string>(),
             std::back_inserter<vector<string>>(items));

        TermID termID;
        TermData data;
        istringstream(items[0]) >> termID;
        istringstream(items[1]) >> data.idf;
        istringstream(items[2]) >> data.totalFreq;
        istringstream(items[3]) >> data.postingIndex;
        istringstream(items[4]) >> data.postingBit;
        addTerm(termID, data);
    }

    cerr << "[Lexicon]: added " << _entries.size() << " entries" << endl;
}

unsigned int Lexicon::getDocLength(DocID docID) const
{
    return _docLengths.at(docID);
}

unsigned int Lexicon::getNumDocs() const
{
    return _docLengths.size();
}

double Lexicon::getAvgDocLength() const
{
    return _avgDL;
}

void Lexicon::setAvgDocLength()
{
    double sum = 0;
    for(auto & length: _docLengths)
        sum += length.second;

    _avgDL = static_cast<double>(sum) / _docLengths.size();
}

string Lexicon::getTerm(TermID termID) const
{
    return _termMap.getValueByKey(termID);
}

TermID Lexicon::getTermID(string term) const
{
    return _termMap.getKeyByValue(term);
}

bool Lexicon::containsTermID(TermID termID) const
{
    return termID < _termMap.size();
}

string Lexicon::getDoc(DocID docID) const
{
    return _docMap.getValueByKey(docID);
}

DocID Lexicon::getDocID(string docName) const
{
    return _docMap.getKeyByValue(docName);
}

const InvertibleMap<TermID, string> & Lexicon::getTermIDMapping() const
{
    return _termMap;
}

void Lexicon::setDocLengths(const string & filename)
{
    Parser parser(filename, " \n");
    while(parser.hasNext())
    {
        DocID docID;
        unsigned int length;
        istringstream(parser.next()) >> docID;
        istringstream(parser.next()) >> length;
        _docLengths.insert(make_pair(docID, length));
    }
}

}
}
