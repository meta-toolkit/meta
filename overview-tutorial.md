---
title: MeTA System Overview
layout: page
category: tut
order: 1
---

## MeTA's goal

Our task is to improve upon and complement the current body of open source
machine learning and information retrieval software. The existing environment of
this open source software tends to be quite fragmented.

There is rarely a single location for a wide variety of algorithms; a good
example of this is the [LIBLINEAR](http://www.csie.ntu.edu.tw/~cjlin/liblinear/)
software package for SVMs. While this is the most cited of the open source
implementations of linear SVMs, it focuses solely on kernel-less methods. If
presented with a nonlinear classification problem, one would be forced to find a
different software package that supports kernels (such as the same authors’
[LIBSVM](http://www.csie.ntu.edu.tw/~cjlin/libsvm/) package). This places an
undue burden on the researchers---not only are they required to have a detailed
understanding of the research problem at hand, but they are now forced to
understand this fragmented nature of the open source software community, find
the appropriate tools in this mishmash of implementations, and compile and
configure the appropriate tool.

Even when this is all done, there is the problem of data formatting---it is
unlikely that the tools have standardized upon a single input format, so a
certain amount of "data munging" is now required. This all detracts from the
actual research task at hand, which has a marked impact on the speed of
discovery.

**We want to address these issues.** In particular, we want to provide a
unifying framework for text indexing and analysis methods, allowing researchers
to quickly run controlled experiments. We want to modularize the feature
generation, instance representation, data storage formats, and algorithm
implementations; this will allow for researchers to make seamless transitions
along any of these dimensions with minimal effort.

## How does MeTA store data?

All processed data is stored in an `index`. Currently, we have two indexes:
`forward_index` and `inverted_index`. The former is keyed by document IDs, and
the latter is keyed by term IDs.

 - `forward_index` is used for applications such as topic modeling and
   most classification tasks
 - `inverted_index` is used to create search engines, or do classification with
   *k*-nearest-neighbor

Since each MeTA application takes an index as input, all processed data is
interchangeable between all the components. This also gives a great advantage to
classification: **MeTA supports out-of-core classification by default!** If your
dataset is small enough (like most other toolkits assume), you can use a cache
such as `no_evict_cache` to keep it all in memory without sacrificing any speed.

Index usage is explained in much more detail in the [Search
Tutorial]({{site.baseurl}}/index-tutorial.html), though it might make more sense
to read the about [Filters and
Analyzers]({{site.baseurl}}/analyzers-filters-tutorial.html) first.

## Corpus input formats

There are currently three corpus input formats:

 - `line_corpus`: each dataset consists of one to three files:
   * `corpusname.dat`: each document appears on one line.
   * `corpusname.dat.names`: optional file that includes the name or path of the
      document on each line, corresponding to the order in `corpusname.dat`.
      These are the names that (*e.g.*) are returned when running the search
      engine.
   * `corpusname.dat.labels`: optional file that includes the class or label of
      the document on each line, again corresponding to the order in
      `corpusname.dat`. These are the labels that are used for the
      classification tasks.
 - `gz_corpus`: similar to `line_corpus`, but each of its files and
    metadata files are compressed using gzip compression:
    * `corpusname.dat.gz`: compressed version of `corpusname.dat`
    * `corpusname.dat.names.gz`: compressed version of
      `corpusname.dat.names`
    * `corpusname.dat.labels.gz`: compressed version of
      `corpusname.dat.labels`
    * `corpusname.dat.numdocs`: an uncompressed file containing only the
       count of the number of documents in the corpus (this is used to be
       able to provide progress reporting when tokenizing documents)
 - `file_corpus`: each document is its own file, and the name of the file
   becomes the name of the document. There is also a `corpusname-full-corpus.txt`
   which contains (on each line) an optional label for each document followed
   by the path to the file on disk.

If only being used for classification, MeTA can also take libsvm-formatted
input to create a `forward_index`.

## Datasets

There are several public datasets that we've converted to the `line_corpus`
format:

 - [20newsgroups](http://web.engr.illinois.edu/~massung1/files/20newsgroups.tar.gz),
   originally from [here](http://qwone.com/~jason/20Newsgroups/).
 - [IMDB Large Movie Review
   Dataset](http://web.engr.illinois.edu/~massung1/files/imdb.tar.gz),
   originally from [here](http://ai.stanford.edu/~amaas/data/sentiment/).
 - Any [libsvm-formatted
   dataset](http://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/) can be
   used to create a `forward_index`.

## Tokenizers, Filters, and Analyzers

MeTA processes data with a system of tokenizers, filters and analyzers before it
is indexed.

**Tokenizers** come first. They define how to split a document's content into
tokens. Some examples are:

 - `icu_tokenizer`: converts documents into streams of tokens by following the
   unicode standards for sentence and word segmentation.
 - `character_tokenizer`: converts documents into streams of single characters.

**Filters** come next, and can be chained together. They define ways that text
can be modified or transformed. Here are some examples of filters:

 - `length_filter`: this filter accepts tokens that are within a certain length
   and rejects those that are not.
 - `icu_filter`: applies an [ICU](http://userguide.icu-project.org/)
   transliteration to each token in the sequence.
 - `list_filter`: this filter either accepts or rejects tokens based on a list.
   For example, you could use a stopword list and reject stopwords.
 - `porter2_stemmer`: this filter transforms each token according to the
   [Porter2 English
   Stemmer](http://snowball.tartarus.org/algorithms/english/stemmer.html) rules.

**Analyzers** operate on the output from the filter chain and produce token
counts from documents. Here are some examples of analyzers:

 - `ngram_word_analyzer`: collects and counts sequences of *n* words (tokens)
   that have been filtered by the filter chain
 - `ngram_pos_analyzer`: same as `ngram_word_analyzer`, but operates on
   part-of-speech tags from MeTA's CRF
 - `tree_analyzer`: collects and counts occurrences of parse tree structural
   features, currently the only known implementation of work described in [this
   paper](http://web.engr.illinois.edu/~massung1/files/icsc-2013.pdf).
 - `libsvm_analyzer`: converts a libsvm `line_corpus` into MeTA format.

For a more detailed intro, see [Filters and
Analyzers]({{site.baseurl}}/analyzers-filters-tutorial.html).

## Unit tests

We're using [ctest](http://www.cmake.org/cmake/help/v2.8.8/ctest.html), which
is configured to run all the tests available in the `unit-test.cpp` file.
You may run

{% highlight bash %}
ctest --output-on-failure
{% endhighlight %}

to execute the unit tests while displaying output from failed tests.

The file `unit-test.cpp`, takes various tests as parameters. For example,

{% highlight bash %}
./unit-test inverted-index
{% endhighlight %}

or

{% highlight bash %}
./unit-test all
{% endhighlight %}

Please note that you must have a valid configuration file (config.toml) in the
project root, since many unit tests create input based on paths stored there.

You won't normally have to worry about running `./unit-test` yourself since we
have ctest set up. However, since ctest creates a new process for each test, it
is hard to debug. If you'd like to debug with tools such as
[valgrind](http://valgrind.org/) or [gdb](http://www.sourceware.org/gdb/),
running `./unit-test` manually is a better option.

## Bugs

On GitHub, create an issue and make sure to label it as a bug. Please be as
detailed as possible. Including a code snippet would be very helpful if
applicable.

You many also add feature requests the same way, though of course the
developers will have their own priority for which features are addressed first.

## Contribute

Forks are certainly welcome. When you have something you'd like to add, submit
a pull request and a developer will approve it after review.

We're also looking for more core developers. If you're interested, don't
hesitate to contact us!

## Citing MeTA

For now, please cite the homepage (<http://meta-toolkit.github.io/meta/>) and
note which revision you used.

Once (hopefully) we have a paper, you would be able to cite that.

## Developers

[Sean Massung](http://web.engr.illinois.edu/~massung1/) started writing MeTA in
May 2012 as a tool to help with his senior thesis at UIUC. He is currently a
graduate student there, working on information retrieval and natural language
processing with [Professor ChengXiang
Zhai](http://www.cs.uiuc.edu/homes/czhai/).

[Chase Geigle](https://chara.cs.illinois.edu/sites/cgeigle/) needs to write
his bio.

## License

MeTA is dual-licensed under the [MIT
License](http://opensource.org/licenses/MIT) and the [University of
Illinois/NCSA Open Source License](http://opensource.org/licenses/NCSA).
These are free, permissive licenses, meaning it is allowed to be included
in proprietary software.
