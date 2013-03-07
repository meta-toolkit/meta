**********************************************************
SUPERVISED LATENT DIRICHLET ALLOCATION FOR CLASSIFICATION
**********************************************************

(C) Copyright 2009, Chong Wang, David Blei and Li Fei-Fei

written by Chong Wang, chongw@cs.princeton.edu, part of code
is from http://www.cs.princeton.edu/~blei/lda-c/index.html.

This file is part of slda.

slda is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

slda is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
USA


------------------------------------------------------------------------

This is a C++ implementation of supervised latent Dirichlet allocation (sLDA) 
for classification.

Note that this code requires the Gnu Scientific Library, http://www.gnu.org/software/gsl/

------------------------------------------------------------------------


TABLE OF CONTENTS


A. COMPILING

B. ESTIMATION

C. INFERENCE


------------------------------------------------------------------------

A. COMPILING

Type "make" in a shell. Make sure the GSL is installed.


------------------------------------------------------------------------

B. ESTIMATION

Estimate the model by executing:

     slda [est] [data] [label] [settings] [alpha] [k] [seeded/random/model_path] [directory]

The saved models are in two files:

     <iteration>.model is the model saved in the binary format, which is easy and
     fast to use for inference.

     <iteration>.model.txt is the model saved in the text format, which is
     convenient for printing topics or analysis using python.
     

The variational posterior Dirichlets are in:

     <iteration>.gamma


Data format

(1) [data] is a file where each line is of the form:

     [M] [term_1]:[count] [term_2]:[count] ...  [term_N]:[count]

where [M] is the number of unique terms in the document, and the
[count] associated with each term is how many times that term appeared
in the document. 

(2) [label] is a file where each line is the corresponding label for [data].
The labels must be 0, 1, ..., C-1, if we have C classes.


------------------------------------------------------------------------

C. INFERENCE

To perform inference on a different set of data (in the same format as
for estimation), execute:

     slda [inf] [data] [label] [settings] [model] [directory]
    
where [model] is the binary file from the estimation.
     
The predictive labels are in:

     inf-labels.dat

The variational posterior Dirichlets are in:

     inf-gamma.dat

