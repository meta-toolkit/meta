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

#include <fstream>
#include <sstream>
#include <string>

struct settings
{
    using string = std::string;

    float  var_converged;
    int    var_max_iter;
    float  em_converged;
    int    em_max_iter;
    int    estimate_alpha;
    float  penalty;
    float  alpha;
    int    num_topics;
    string init_method;

    settings(const string & filename)
    {
        std::ifstream infile(filename);
        string line;
        while(infile >> line)
        {
            if(line == "var-max-iter")         infile >> var_max_iter;
            else if(line == "var-convergence") infile >> var_converged;
            else if(line == "em-max-iter")     infile >> em_max_iter;
            else if(line == "em-convergence")  infile >> em_converged;
            else if(line == "L2-penalty")      infile >> penalty;
            else if(line == "alpha-val")       infile >> alpha;
            else if(line == "num-topics")      infile >> num_topics;
            else if(line == "init-method")     infile >> init_method;
            else if(line == "alpha-action")
            {
                string opt;
                infile >> opt;
                estimate_alpha = (opt == "fixed") ? 0 : 1;
            }
        }
        infile.close();
    }
};

#endif // SETTINGS_H

