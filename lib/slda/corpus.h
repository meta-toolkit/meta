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

#ifndef CORPUS_H
#define CORPUS_H

#include <vector>
#include <stddef.h>

class document
{
public:
    int * words;
    int * counts;
    int length;
    int total;
    int label;

    document()
    {
        words = NULL;
        counts = NULL;
        length = 0;
        total = 0;
        label = -1;
    }
    document(int len)
    {
        length = len;
        words = new int [length];
        counts = new int [length];
        total = 0;
        label = -1;
    }
    ~document()
    {
        if (words != NULL)
        {
            delete [] words;
            delete [] counts;
            length = 0;
            total = 0;
            label = -1;
        }
    }
};

class corpus
{
public:
    corpus();
    ~corpus();
    void read_data(const char * data_filename, const char * label_filename);
    int max_corpus_length();

    int num_docs;
    int size_vocab;
    int num_classes;
    int num_total_words;
    std::vector<document*> docs;
};

#endif // CORPUS_H
