/**
 * @file structs.cpp
 */

#include "util/common.h"
#include "index/document.h"
#include "index/structs.h"

using std::istringstream;
using std::string;
using std::vector;

IndexEntry::IndexEntry(const string & str):
    data(vector<PostingData>())
{
    istringstream line(str);
    vector<string> items;

    std::copy(std::istream_iterator<string>(line),
         std::istream_iterator<string>(),
         std::back_inserter<vector<string>>(items));

    istringstream(items[0]) >> termID;

    for(size_t i = 1; i < items.size(); i += 2)
    {
        DocID docID;
        unsigned int freq;
        istringstream(items[i]) >> docID;
        istringstream(items[i + 1]) >> freq;
        data.push_back(PostingData(docID, freq));
    }
}

bool IndexEntry::operator<(const IndexEntry & other) const
{
    return termID < other.termID;
}

string IndexEntry::toString() const
{
    string str = Common::toString(termID);
    for(auto & pd: data)
    {
        str += " " + Common::toString(pd.docID);
        str += " " + Common::toString(pd.freq);
    }
    return str;
}

bool PostingData::operator<(const PostingData & other) const
{
    return docID < other.docID;
}
