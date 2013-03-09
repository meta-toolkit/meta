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

#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include "corpus.h"
#include "utils.h"
#include "slda.h"

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::pair;

int main(int argc, char* argv[])
{
    if(argc < 5 || argc > 6)
    {
        cerr << "usage: " << argv[0]
             << " [est] [data] [settings] [directory]"
             << endl << "       " << argv[0]
             << " [inf] [data] [settings] [model] [directory]" << endl;
        return 1;
    }

    std::vector<string> args(argv + 1, argv + argc);

    if(args[0] != "est" && args[0] != "inf")
    {
        cerr << "arg 1 must be \"est\" or \"inf\"" << endl;
        return 1;
    }

    corpus c;
    string data_filename = args[1];
    c.read_data(data_filename);
    settings setting(args[2]);

    if(args[0] == "est")
    {
        string directory = args[3];
        make_directory(directory);

        slda model;
        model.init(setting.alpha, setting.num_topics, &c);
        model.v_em(&c, &setting, setting.init_method, directory);
    }

    if(args[0] == "inf")
    {
        string directory = args[4];
        make_directory(directory);

        string model_filename = args[3];
        slda model;
        model.load_model(model_filename);
        model.infer_only(&c, &setting, directory);

        vector<vector<pair<int, double>>> dists = model.top_terms();
        size_t d = 0;
        for(auto & dist: dists)
        {
            cout << "Top terms for dist " << d++ << endl;
            for(size_t i = 0; i < 10; ++i)
                cout << "  " << dist[i].first << " " << dist[i].second << endl;
        }
    }

    return 0;
}
