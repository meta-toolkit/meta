// (C) Copyright 2009, Chong Wang, David Blei and Li Fei-Fei

// written by Chong Wang, chongw@cs.princeton.edu

// This file is part of slda.

// slda is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your
// option) any later version.

// slda is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <assert.h>
#include <stdio.h>
#include "corpus.h"

using std::pair;
using std::vector;
using std::string;
using std::cout;
using std::endl;

corpus::corpus()
{
    size_vocab = 0;
    num_classes = 0;
    num_total_words = 0;
}

corpus::~corpus()
{
    for (size_t i = 0; i < docs.size(); i ++)
        delete docs[i];
}

void corpus::read_data(const string & data_filename)
{
    std::ifstream infile(data_filename);
    string line;
    while(std::getline(infile, line))
    {
        size_t length = count_if(line.begin(), line.end(), [](unsigned char ch){
            return std::isspace(ch); });

        document* doc = new document(length - 1);
        doc->words.reserve(length);
        doc->counts.reserve(length);

        std::stringstream stream(line);
        
        stream >> doc->label;
        if(doc->label >= num_classes)
            num_classes = doc->label + 1;

        // stream now contains "word_id:count" strings
        string token;
        while(stream >> token)
        {
            pair<int, int> vals = get_word_data(token);
            int word = vals.first;
            int count = vals.second;
            doc->words.push_back(word);
            doc->counts.push_back(count);
            doc->total += count;

            if(word >= size_vocab)
                size_vocab = word + 1;

            num_total_words += doc->total;
        }

        docs.push_back(doc);
    }
    
    infile.close();

    cout << "number of docs  : " << docs.size() << endl;
    cout << "number of terms : " << size_vocab << endl;
    cout << "number of total words: " << num_total_words << endl;
    cout << "number of classes : " << num_classes << endl;
}

pair<int, int> corpus::get_word_data(const string & str) const
{
    size_t idx = str.find_first_of(':');
    std::istringstream wordstr(str.substr(0, idx));
    std::istringstream countstr(str.substr(idx + 1));

    int word = 0;
    int count = 0;
    wordstr >> word;
    countstr >> count;

    return std::make_pair(word, count);
}

int corpus::max_corpus_length() {
    int max_length = 0;
    for (size_t d = 0; d < docs.size(); d++) {
        if (docs[d]->length > max_length)
            max_length = docs[d]->length;
    }
    return max_length;
}
