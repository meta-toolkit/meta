#include "chunk_list.h"

ChunkList::ChunkList(size_t numChunks):
    _numChunks(numChunks),
    _parsers(vector<Parser>())
{
    for(size_t i = 0; i < _numChunks; ++i)
    {
        string filename = Common::toString(i) + ".chunk";
        _parsers.push_back(Parser(filename, "\n"));
    }

    hasNext();
}

bool ChunkList::hasNext() const
{
    cerr << "[ChunkList]: hasNext() called " << _parsers.size() << " " << _parsers[0].hasNext() << endl;
    return _parsers[0].hasNext();
}

void ChunkList::combinePostingData(vector<IndexEntry> & entries, IndexEntry & combined) const
{
    for(auto & entry: entries)
    {
        for(auto & pd: entry.data)
            combined.data.push_back(std::move(pd));
    }
    std::sort(combined.data.begin(), combined.data.end());
    entries.clear();
}

IndexEntry ChunkList::next()
{
    if(!_parsers[0].hasNext())
    {
        cerr << "[ChunkList]: tried to getNext() when there were no more elements" << endl;
        return IndexEntry(0);
    }

    IndexEntry min(_parsers[0].peek());

    // change this.........

    for(size_t i = 1; i < _numChunks; ++i)
    {
        IndexEntry next(_parsers[i].peek());
        if(next < min)
            min = next;
    }

    cerr << "[ChunkList]: min == " << min.termID << endl;

    vector<IndexEntry> mins;
    for(size_t i = 0; i < _numChunks; ++i)
    {
        if(IndexEntry(_parsers[i].peek()).termID == min.termID)
        {
            mins.push_back(IndexEntry(_parsers[i].next()));
            if(!_parsers[i].hasNext())
            {
                std::swap(_parsers[i], _parsers[_numChunks - 1]);
                --_numChunks;
                --i;
                if(_numChunks == 0)
                    break;
            }
        }
    }

    if(mins.size() == 1)
        return mins[0];

    IndexEntry combined(mins[0].termID);
    combinePostingData(mins, combined);
    return combined;
}
