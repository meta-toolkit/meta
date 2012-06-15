/**
 * @file lexicon.cpp
 */

#include "lexicon.h"

Lexicon::Lexicon(const string & lexiconFile):
    _lexiconFilename(lexiconFile)
{
    _entries = new unordered_map<TermID, TermData>();
    readLexicon();
}

Lexicon::~Lexicon()
{
    delete _entries;
}

Lexicon::Lexicon(const Lexicon & other)
{
    _entries = new unordered_map<TermID, TermData>(*other._entries);
    _lexiconFilename = other._lexiconFilename;
}

const Lexicon & Lexicon::operator=(const Lexicon & other)
{
    if(this != &other)
    {
        delete _entries;
        _entries = other._entries;
        _lexiconFilename = other._lexiconFilename;
    }
    return *this;
}

TermData Lexicon::getTermInfo(TermID termID) const
{
    unordered_map<TermID, TermData>::const_iterator it = _entries->find(termID);
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
        unordered_map<TermID, TermData>::const_iterator it;
        for(it = _entries->begin(); it != _entries->end(); ++it)
        {
            string line = toString(it->first) + " ";
            TermData data = it->second;
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

template <class T>
string Lexicon::toString(T value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
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
    while(parser.hasNext())
    {
        istringstream line(parser.next());
        vector<string> items;
        copy(std::istream_iterator<string>(line),
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
