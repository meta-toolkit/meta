#include "chunk_list.h"

bool PostingData::operator<(const PostingData & other) const
{
    return docID < other.docID;
}

ChunkList::ChunkList(size_t numChunks):
    _numChunks(numChunks),
    _parsers(vector<Parser>())
{
    for(size_t i = 0; i < numChunks; ++i)
        _parsers.push_back(Parser(Common::toString(i) + ".chunk", " \n"));
}

bool ChunkList::hasNext() const
{
    return !_parsers.empty();
}

string ChunkList::next()
{

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
