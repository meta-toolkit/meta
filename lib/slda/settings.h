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

struct settings
{
    float VAR_CONVERGED;
    int   VAR_MAX_ITER;
    float EM_CONVERGED;
    int   EM_MAX_ITER;
    int   ESTIMATE_ALPHA;
    float PENALTY;

    void read_settings(char* filename)
    {
        FILE * fileptr;
        char alpha_action[100];

        fileptr = fopen(filename, "r");
        fscanf(fileptr, "var max iter %d\n", &this->VAR_MAX_ITER);
        fscanf(fileptr, "var convergence %f\n", &this->VAR_CONVERGED);
        fscanf(fileptr, "em max iter %d\n", &this->EM_MAX_ITER);
        fscanf(fileptr, "em convergence %f\n", &this->EM_CONVERGED);
        fscanf(fileptr, "L2 penalty %f\n", &this->PENALTY);

        fscanf(fileptr, "alpha %s", alpha_action);
        if (strcmp(alpha_action, "fixed") == 0)
        {
            this->ESTIMATE_ALPHA = 0;
            printf("alpha is fixed ...\n");
        }
        else
        {
            this->ESTIMATE_ALPHA = 1;
            printf("alpha is esimated ...\n");
        }
        fclose(fileptr);
        printf("var max iter %d\n", this->VAR_MAX_ITER);
        printf("var convergence %.2E\n", this->VAR_CONVERGED);
        printf("em max iter %d\n", this->EM_MAX_ITER);
        printf("em convergence %.2E\n", this->EM_CONVERGED);
        printf("L2 penalty %.2E\n", this->PENALTY);
    }
};

#endif // SETTINGS_H

