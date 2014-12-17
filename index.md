---
title: MeTA
layout: default
---

## MeTA is a modern C++ data sciences toolkit featuring

 - text tokenization, including deep semantic features like parse trees
 - inverted and forward indexes with compression and various caching strategies
 - a collection of ranking functions for searching the indexes
 - topic models
 - classification algorithms
 - graph algorithms
 - language models
 - CRF implementation (POS-tagging, shallow parsing)
 - wrappers for liblinear and libsvm (including libsvm dataset parsers)
 - UTF8 support for analysis on various languages
 - multithreaded algorithms

## Documentation

Doxygen documentation can be found [here]({{site.baseurl}}/doxygen/).

## Tutorials

We have walkthroughs for the following parts of MeTA:

 - [System Overview]({{site.baseurl}}/overview-tutorial.html)
 - [Analyzers and Filters]({{site.baseurl}}/analyzers-filters-tutorial.html)
 - [Search]({{site.baseurl}}/search-tutorial.html)
 - [Classifiers]({{site.baseurl}}/classify-tutorial.html)
 - [Topic Models]({{site.baseurl}}/topic-models-tutorial.html)

## Project setup

 - This project requires a conforming C++11 compiler. Currently, clang >=
   3.3 is the de-facto compiler for use with this project, but it will work
   with GCC >= 4.8 as of writing.

 - Additionally, you will need a conforming implementation of the C++11
   standard library and ABI---currently libc++ and libc++abi are the best
   options for this if you are using clang. These are installed by default
   on OS X, but may need to be installed as separate packages on other
   systems (e.g., Arch). See your distribution's package manager for more
   information on installing these dependencies.

   If you are using GCC >= 4.8, its packaged standard library and ABI is
   sufficient.

 - Windows users: YMMV. It is not currently supported, but things may
   work. You will likely need Visual Studio 2013 for the C++11 features.

 - This project makes use of several [git
   submodules](http://git-scm.com/book/en/Git-Tools-Submodules). Instructions on
   how to use them is included in the code snippet below.

 - Make sure you compile liblinear and libsvm (located in deps/libsvm-modules)
   if you want to use them. It is not necessary to compile or modify any other
    submodules.

 - To get started, run the following commands in the root of the project:

{% highlight bash %}
# checkout the project
git clone https://github.com/meta-toolkit/meta.git
cd meta/

# set up submodules
git submodule init
git submodule update

mkdir build
cd build/

# to use clang
CXX=clang++ cmake ../ -DCMAKE_BUILD_TYPE=Release
# OR
# to use gcc
CXX=g++ cmake ../ -DCMAKE_BUILD_TYPE=Release

make
{% endhighlight %}

 - There are rules for clean, tidy, and doc. **After you run the cmake command
   once, you will be able to just run `make` as usual** when you're
   developing---it'll detect when the CMakeLists.txt file has changed and
   rebuild Makefiles if it needs to.

 - To compile in debug mode, just replace `Release` with `Debug`.

---

[Home]({{site.baseurl}}/)
&nbsp; | &nbsp;
[Overview]({{site.baseurl}}/overview-tutorial.html)
&nbsp; | &nbsp;
[Analyzers and Filters]({{site.baseurl}}/analyzers-filters-tutorial.html)
&nbsp; | &nbsp;
[Search]({{site.baseurl}}/search-tutorial.html)
&nbsp; | &nbsp;
[Classifiers]({{site.baseurl}}/classify-tutorial.html)
&nbsp; | &nbsp;
[Topic Models]({{site.baseurl}}/topic-models-tutorial.html)
