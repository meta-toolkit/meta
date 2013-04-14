/**
 * @file chunk_list.cpp
 */

#include <string>
#include <iostream>
#include "io/parser.h"
#include "index/structs.h"
#include "util/common.h"
#include "index/index.h"
#include "index/chunk_list.h"

namespace meta {
namespace index {

using std::cerr;
using std::endl;
using std::vector;
using std::string;

using io::Parser;

ChunkList::ChunkList(size_t numChunks):
    _numChunks(numChunks),
    _parsers(vector<Parser>())
{
    for(size_t i = 0; i < _numChunks; ++i)
    {
        string filename = common::to_string(i) + ".chunk";
        _parsers.push_back(Parser(filename, "\n"));
    }
}

bool ChunkList::hasNext() const
{
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
        throw Index::index_exception("[ChunkList]: tried to getNext() when there were no more elements");

    IndexEntry min(_parsers[0].peek());

    // change this.........
    // can I assume the minimums will be 0,1,2,3,....

    for(size_t i = 1; i < _numChunks; ++i)
    {
        IndexEntry next(_parsers[i].peek());
        if(next < min)
            min = next;
    }

    vector<IndexEntry> mins;
    for(size_t i = 0; i < _numChunks; ++i)
    {
        // see if it has the min value
        IndexEntry current(_parsers[i].peek());
        if(current.termID == min.termID)
        {
            string next = _parsers[i].next();
            mins.push_back(IndexEntry(next));
        }

        // get to valid spot
        if(!_parsers[i].hasNext())
        {
            cerr << " -> used up " << _parsers[i].filename() << endl;
            std::swap(_parsers[i], _parsers[_numChunks - 1]);
            --_numChunks;
            if(_numChunks == 0)
                break;
        }
    }

    if(mins.size() == 1)
        return mins[0];

    IndexEntry combined(mins[0].termID);
    combinePostingData(mins, combined);
    return combined;
}

}
}
