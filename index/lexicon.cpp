/**
 * @file lexicon.cpp
 */

#include "lexicon.h"

Lexicon::Lexicon(const string & lexiconFile): _lexiconFilename(lexiconFile)
{
    _entries = new unordered_map<TermID, TermData>();
    _docLengths = new unordered_map<DocID, unsigned int>();
    readLexicon();
    readDocLengths();
    setAvgDocLength();
}

Lexicon::~Lexicon()
{
    clear();
}

Lexicon::Lexicon(const Lexicon & other)
{
    copy(other);
}

const Lexicon & Lexicon::operator=(const Lexicon & other)
{
    if(this != &other)
    {
        clear();
        copy(other);
    }
    return *this;
}

bool Lexicon::isEmpty() const
{
    return _entries->empty();
}

TermData Lexicon::getTermInfo(TermID termID) const
{
    auto it = _entries->find(termID);
    if(it == _entries->end())
    {
        cerr << "[Lexicon]: warning: termID lookup failed" << endl;
        return TermData();
    }
    return it->second;
}

void Lexicon::save() const
{
    ofstream outfile(_lexiconFilename);
    if(outfile.good())
    {
        // save the paths to the doclengths and mappings
        outfile << _lengthsFilename << endl;

        string termMapFilename = "termid.mapping";
        string docMapFilename = "docid.mapping";
        outfile << termMapFilename << endl;
        outfile << docMapFilename << endl;

        saveMap(termMapFilename, *_termMap);
        saveMap(docMapFilename, *_docMap);

        for(auto & entry: *_entries)
        {
            string line = toString(entry.first) + " ";
            TermData data = entry.second;
            line += toString(data.idf) + " ";
            line += toString(data.totalFreq) + " ";
            line += toString(data.postingIndex) + " ";
            line += toString(data.postingBit) + "\n";
            outfile << line;
        }
        outfile.close();
    }
    else
    {
        cerr << "[Lexicon]: error writing lexicon to disk" << endl;
    }
}

void Lexicon::addTerm(TermID term, TermData termData)
{
    _entries->insert(make_pair(term, termData));
}

void Lexicon::readLexicon()
{
    Parser parser(_lexiconFilename, "\n");
    if(!parser.isValid())
    {
        cerr << "[Lexicon]: created empty lexicon" << endl;
        return;
    }

    cerr << "[Lexicon]: reading from file..." << endl;

    // the first, second, and third lines in the lexicon file correspond
    //  to the doclengths files, term id mapping, and docid mapping files respectively
    _lengthsFilename = parser.next();
    setTermMap(parser.next());
    setDocMap(parser.next());

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
}

void Lexicon::readDocLengths()
{
    Parser parser(_lengthsFilename, " \n");

    while(parser.hasNext())
    {
        DocID docID;
        unsigned int length;
        istringstream(parser.next()) >> docID;
        istringstream(parser.next()) >> length;
        _docLengths->insert(make_pair(docID, length));
    }
}

unsigned int Lexicon::getDocLength(DocID docID) const
{
    return _docLengths->at(docID);
}

unsigned int Lexicon::getNumDocs() const
{
    return _docLengths->size();
}

double Lexicon::getAvgDocLength() const
{
    return _avgDL;
}

void Lexicon::setAvgDocLength()
{
    double sum = 0;
    for(auto & length: *_docLengths)
        sum += length.second;

    _avgDL = (double) sum / _docLengths->size();
}

string Lexicon::getTerm(TermID termID) const
{
    return _termMap->getValueByKey(termID);
}

TermID Lexicon::getTermID(string term) const
{
    return _termMap->getKeyByValue(term);
}

string Lexicon::getDoc(DocID docID) const
{
    return _docMap->getValueByKey(docID);
}

DocID Lexicon::getDocID(string docName) const
{
    return _docMap->getKeyByValue(docName);
}

void Lexicon::setTermMap(const string & filename)
{
    Parser parser(filename, " \n");
    while(parser.hasNext())
    {
        TermID termID;
        istringstream(parser.next()) >> termID;
        _termMap->insert(termID, parser.next());
    }
}

void Lexicon::setDocMap(const string & filename)
{
    Parser parser(filename, " \n");
    while(parser.hasNext())
    {
        DocID docID;
        istringstream(parser.next()) >> docID;
        _termMap->insert(docID, parser.next());
    }
}

void Lexicon::clear()
{
    delete _entries;
    delete _docLengths;
    delete _termMap;
    delete _docMap;
}

void Lexicon::copy(const Lexicon & other)
{
    _lexiconFilename = other._lexiconFilename;
    _lengthsFilename = other._lengthsFilename;

    // other._thing may be empty, but will never be NULL
    _entries = new unordered_map<TermID, TermData>(*other._entries);
    _docLengths = new unordered_map<DocID, unsigned int>(*other._docLengths);
    _termMap = new InvertibleMap<TermID, string>(*other._termMap);
    _docMap = new InvertibleMap<DocID, string>(*other._docMap);
}

template <class KeyType>
void Lexicon::saveMap(const string & filename, const InvertibleMap<KeyType, string> & map) const
{
    ofstream outfile(_lexiconFilename);
    if(outfile.good())
    {
        for(auto & entry: map)
            outfile << toString(entry.first) << " " << entry.second << endl;
        outfile.close();
    }
    else
    {
        cerr << "[Lexicon]: error writing map to disk" << endl;
    }

}

void Lexicon::createFromPostings(const string & filename)
{

}

void Lexicon::createFromCompressedPostings(const string & filename)
{

}

template <class T>
string Lexicon::toString(T value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}
