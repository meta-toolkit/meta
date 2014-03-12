---
layout: default
title: Tutorial - Search
---

As mentioned in the overview, at the heart of MeTA lies its indexes. Every
piece of data that MeTA's algorithms operate on must first reside in an index.
In this tutorial, we'll walk you through how to create your own
`inverted_index` and `forward_index`, and how to use your `inverted_index` to
create a search engine.

## Using the config file and the example apps

In the root directory of MeTA, there are several example apps that deal with
creating indexes for search engines or information retrieval tasks:

 - `index.cpp`: indexes a corpus
 - `search.cpp`: searches for documents that already exist in the corpus
 - `interactive-search.cpp`: searches for documents based on user input
 - `query-runner.cpp`: runs a list of queries specified by a file

There is another app that deal with classification: `classify.cpp`.

In this tutorial, we'll mainly focus on creating an `inverted_index` for making
a search engine. The [Classifiers]({{site.baseurl}}/classify-tutorial.html)
tutorial deals with how to use (mainly) `forward_index`s to run machine learning
algorithms, though we will walk through how to create a `forward_index`.

Say what needs to be in `config.toml`.

How to compile?

## Using the index typedefs

Use these common pre-specified indexes.

```cpp
using namespace meta;
auto idx = make_index<index::memory_inverted_index>(config);  // 1
auto idx = make_index<index::inverted_index>(config);         // 2
auto idx = make_index<index::memory_forward_index>(config);   // 3
auto idx = make_index<index::forward_index>(config);          // 4
```

Code snippet to create an index and search.

## Specifying your own cache

More control over your index.

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
