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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdio.h>
#include <string.h>
#include <string>

struct settings
{
    float VAR_CONVERGED;
    int   VAR_MAX_ITER;
    float EM_CONVERGED;
    int   EM_MAX_ITER;
    int   ESTIMATE_ALPHA;
    float PENALTY;
    float ALPHA;
    int   NUM_TOPICS;

    void read_settings(const std::string & filename)
    {
        FILE * fileptr;
        char alpha_action[100];

        fileptr = fopen(filename.c_str(), "r");
        fscanf(fileptr, "var max iter %d\n", &VAR_MAX_ITER);
        fscanf(fileptr, "var convergence %f\n", &VAR_CONVERGED);
        fscanf(fileptr, "em max iter %d\n", &EM_MAX_ITER);
        fscanf(fileptr, "em convergence %f\n", &EM_CONVERGED);
        fscanf(fileptr, "L2 penalty %f\n", &PENALTY);
        fscanf(fileptr, "alpha val %f\n", &ALPHA);
        fscanf(fileptr, "num topics %d\n", &NUM_TOPICS);

        fscanf(fileptr, "alpha %s", alpha_action);
        if (strcmp(alpha_action, "fixed") == 0)
            ESTIMATE_ALPHA = 0;
        else
            ESTIMATE_ALPHA = 1;

        fclose(fileptr);
    }
};

#endif // SETTINGS_H

