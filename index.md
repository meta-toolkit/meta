---
title: MeTA
layout: default
---

## MeTA is a modern C++ data sciences toolkit featuring

 - text tokenization, including deep semantic features like parse trees
 - inverted and forward indexes with compression and various caching strategies
 - various ranking functions for searching the indexes
 - topic modeling algorithms
 - classification algorithms
 - wrappers for liblinear and libsvm
 - UTF8 support for analysis on various languages
 - multithreaded algorithms

Our current goal for MeTA is to publish in [JMLR's Machine Learning Open-Source
Software](http://jmlr.org/mloss/).

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

{% highlight bash %}
git submodule init
git submodule update
{% endhighlight %}

   Then, make sure you compile liblinear and libsvm (located in
   deps/libsvm-modules). It is not necessary to compile or modify any other
   submodules.

 - To compile initially, run the following commands

{% highlight bash %}
mkdir build
cd build
CXX=clang++ cmake ../ -DCMAKE_BUILD_TYPE=Debug
make
{% endhighlight %}

 - There are rules for clean, tidy, and doc. (Also, once you run the cmake
   command once, you should be able to just run make like usual as you're
   developing---it'll detect when the CMakeLists.txt file has changed and
   rebuild Makefiles if it needs to.) To compile in release mode, just replace
   `Debug` with `Release`.

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
