# MeTA: ModErn Text Analysis

Please visit our [web page](http://meta-toolkit.github.io/meta/) for information
about MeTA!

## Overview

MeTA is a modern C++ data sciences toolkit featuring

 - text tokenization, including deep semantic features like parse trees

 - inverted and forward indexes with compression and various caching strategies

 - various ranking functions for the indexes

 - topic modeling algorithms

 - language modeling algorithms

 - clustering and similarity algorithms

 - classification algorithms

 - wrappers for liblinear and slda

Doxygen documentation can be found
[here](http://web.engr.illinois.edu/~massung1/toolkit-doc/). Note that this is
probably not as frequently updated as it should be.

Our current goal for MeTA is to publish in [JMLR's Machine Learning Open-Source
Software](http://jmlr.org/mloss/).

## Build Status (by branch)
- master: [![Build
  Status](https://travis-ci.org/meta-toolkit/meta.png?branch=master)](https://travis-ci.org/meta-toolkit/meta)
- develop: [![Build
  Status](https://travis-ci.org/meta-toolkit/meta.png?branch=develop)](https://travis-ci.org/meta-toolkit/meta)

## Project setup

 - This project requires a very well conforming C++11 compiler. Currently,
   clang is the de-facto compiler for use with this project

 - Additionally, you will need a conformant implementation of the C++11 standard
   library and ABI---currently libc++ and libc++abi are the best options for
   this. See your distribution's package manager for more information on
   installing these dependencies.

 - Windows users: YMMV. It is not currently supported, but things may
   work. You will likely need Visual Studio 2013 for the C++11 features.

 - This project makes use of several [git
   submodules](http://git-scm.com/book/en/Git-Tools-Submodules). To initialize
   these, run
```bash
git submodule init
git submodule update
```

 - Once the submodules are instantiated, go to deps/libsvm-modules and run
   `make` in the liblinear and libsvm directories if you plan on using the
   `svm_wrapper` class.

 - To compile initially, run the following commands
```bash
mkdir build
cd build
# omit CXX=clang++ if you want to use your default compiler
CXX=clang++ cmake ../ -DCMAKE_BUILD_TYPE=Debug
make
```

 - There are rules for clean, tidy, and doc. (Also, once you run the cmake
   command once, you should be able to just run make like usual as you're
   developing---it'll detect when the CMakeLists.txt file has changed and
   rebuild Makefiles if it needs to.)
