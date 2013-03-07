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

#include <stdio.h>
#include <string.h>
#include "corpus.h"
#include "utils.h"
#include "slda.h"

void help( void ) {
    printf("usage: slda [est] [data] [label] [settings] [alpha] [k] [random/seeded/model_path] [directory]\n");
    printf("       slda [inf] [data] [label] [settings] [model] [directory]\n");
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        help();
        return 0;
    }
    if (strcmp(argv[1], "est") == 0)
    {
        corpus c;
        char * data_filename = argv[2];
        char * label_filename = argv[3];
        c.read_data(data_filename, label_filename);
        settings setting;
        char * setting_filename = argv[4];
        setting.read_settings(setting_filename);

        double alpha = atof(argv[5]);
        int num_topics = atoi(argv[6]);
        printf("number of topics is %d\n", num_topics);
        char * init_method = argv[7];
        char * directory = argv[8];
        printf("models will be saved in %s\n", directory);
        make_directory(directory);

        slda model;
        model.init(alpha, num_topics, &c);
        model.v_em(&c, &setting, init_method, directory);
    }

    if (strcmp(argv[1], "inf") == 0)
    {
        corpus c;
        char * data_filename = argv[2];
        char * label_filename = argv[3];
        c.read_data(data_filename, label_filename);
        settings setting;
        char * setting_filename = argv[4];
        setting.read_settings(setting_filename);

        char * model_filename = argv[5];
        char * directory = argv[6];
        printf("\nresults will be saved in %s\n", directory);
        make_directory(directory);

        slda model;
        model.load_model(model_filename);
        model.infer_only(&c, &setting, directory);
    }

    return 0;
}
