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

#include "corpus.h"
#include <assert.h>
#include <stdio.h>

corpus::corpus()
{
    num_docs = 0;
    size_vocab = 0;
    num_classes = 0;
    num_total_words = 0;
}

corpus::~corpus()
{
    for (int i = 0; i < num_docs; i ++)
    {
        document * doc = docs[i];
        delete doc;
    }
    docs.clear();

    num_docs = 0;
    size_vocab = 0;
    num_classes = 0;
    num_total_words = 0;
}

void corpus::read_data(const char * data_filename,
                       const char * label_filename)
{
    int OFFSET = 0;
    int length = 0, count = 0, word = 0,
        n = 0, nd = 0, nw = 0, label = -1;

    FILE * fileptr;
    fileptr = fopen(data_filename, "r");
    printf("\nreading data from %s\n", data_filename);
    nd = 0;
    nw = 0;

    while ((fscanf(fileptr, "%10d", &length) != EOF))
    {
        document * doc = new document(length);
        for (n = 0; n < length; n++)
        {
            fscanf(fileptr, "%10d:%10d", &word, &count);
            word = word - OFFSET;
            doc->words[n] = word;
            doc->counts[n] = count;
            doc->total += count;
            if (word >= nw)
            {
                nw = word + 1;
            }
        }
        num_total_words += doc->total;
        docs.push_back(doc);
        nd++;
    }
    fclose(fileptr);
    num_docs = nd;
    size_vocab = nw;
    printf("number of docs  : %d\n", nd);
    printf("number of terms : %d\n", nw);
    printf("number of total words : %d\n", num_total_words);

    fileptr = fopen(label_filename, "r");
    printf("\nreading labels from %s\n", label_filename);
    nd = 0;
    while ((fscanf(fileptr, "%10d", &label) != EOF))
    {
        document * doc = docs[nd];
        doc->label = label;
        if (label >= num_classes)
        {
            num_classes = label + 1;
        }
        nd ++;
    }
    assert(nd == int(docs.size()));
    printf("number of classes : %d\n\n", num_classes);
}

int corpus::max_corpus_length() {
    int max_length = 0;

    for (int d = 0; d < num_docs; d++) {
        if (docs[d]->length > max_length)
            max_length = docs[d]->length;
    }
    return max_length;
}
