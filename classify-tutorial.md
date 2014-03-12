---
layout: default
title: Tutorial - Classification
---

## Classification in MeTA

Beyond its search and information retrieval capabilities, MeTA also
provides functionality for performing document classification on your
corpora. We will start this tutorial by discussing how you can perform
document classification experiments on your data **without writing a single
line of code**, and then discuss more complicated examples later on.

The first step in setting up your classification task is, of course,
selecting your corpus. MeTA's built-in `classify` program supports the
following corpus inputs:

- anything from which an `inverted_index` can be generated
- pre-processed, libsvm-formatted corpora

MeTA's internal representation for its `forward_index` uses
libsvm-formatted data, so if you already have your data in this format you
are encouraged to use it. Otherwise, you can generate libsvm-formatted
data automatically from any existing `inverted_index`.

## Creating a Forward Index From Scratch

To create a `forward_index` directly from your corpus input, your
configuration file would look something like this:

```toml
corpus-type = "line-corpus"
dataset = "20newsgroups"
list = "20news"
forward-index = "20news-fwd"
inverted-index = "20news-inv"

[[analyzers]]
method = "ngram-word"
ngram = 1
filter = "default-chain"
```

The process looks something like this: first, an `inverted_index` will be
created (or loaded if it already exists) over your corpus' data, and then
it will be un-inverted to create the `forward_index`. The process of
un-inverting is a one-time cost, and is necessary to provide efficient
access to the document vectors for the classifiers. Once you have
generated your `forward_index`, you *never need to generate it again*
unless you want to change your document representation.

## Creating a Forward Index from LIBSVM Data

In many cases, you may already have pre-processed corpora to perform
classification tasks on, and MeTA gracefully handles this. The most common
input for these classification tasks is the LIBSVM file format, and MeTA
supports this format directly as an input corpus.

To create a `forward_index` from data that is already in LIBSVM format,
your configuration file would look something like this:

```toml
corpus-type = "line-corpus"
dataset = "rcv1"
list = "rcv1"
forward-index = "rcv1-fwd"
inverted-index = "rcv1-inv"

[[analyzers]]
method = "libsvm"
```

The `forward_index` will recognize that this is a LIBSVM formatted corpus
and will simply generate a few metadata structures to ensure efficient
random access to document vectors. An `inverted_index` is not created
through this method, and so you will not be able to use classifiers such
as `knn` that require an `inverted_index`.

## Selecting a Classifier

To actually run the `classify` executable, you will need to decide on a
classifier to use. You may see a list of these [in the API documentation
for the `classifier`
class](doxygen/classmeta_1_1classify_1_1classifier.html) (they are listed
as subclasses). The public static id member of each class is the
identifier you would use in the configuration file.

A recommended default configuration is given below, which learns an SVM
via stochastic gradient descent:

```toml
[classifier]
method = "one-vs-all"
    [classifier.base]
    method = "sgd"
    loss = "hinge"
```

Here is an example configuration that uses Naive Bayes:

```toml
[classifier]
method = "naive-bayes"
```

Here is an example that uses k-nearest neighbor with k = 10 and Okapi BM25
as the ranking function:

```toml
[classifier]
method = "knn"
k = 10
    [classifier.ranker]
    method = "bm25"
```

Running `./classify config.toml` from your build directory will now create
a `forward_index` (if necessary) and run 5-fold cross validation on your
data using the prescribed classifier.

## Manual Classification

If you want to customize the classification process (such as providing
your own test/training split, or changing the number of cross-validation
folds), you should interact with the classifiers directly by writing some
code. Refer to `classify.cpp` and [the API documentation for
`classifier](doxygen/classmeta_1_1classify_1_1classifier.html).

## Writing Your Own Classifiers

Any classifiers you write should subclass `classify::classifier` and
implement its virtual methods. You should be clear as to whether your
classifier directly supports multi-class classification (subclass from
`classify::classifier` directly) or only binary classification (sublcass
from `classify::binary_classifier`).

If you would like to be able to create your classifier by specifying it in
a configuration file, you will need to provide a public static id member
that specifies the text that identifies your classifier class, and
register it with the toolkit somewhere in `main()` like this:

```cpp
// if you have a multi-class classifier
meta::classify::register_classifier<my_classifier>();

// if you have a multi-class classifier that requires an
// inverted_index
meta::classify::register_multi_index_classifier<my_classifier>();

// if you have a binary classifier
meta::classify::register_binary_classifier<my_binary_classifier>();
```

If you need to read parameters from the configuration group given for your
classifier, you should specialize the `make_classifier()` function like so:

```cpp
// if you have a multi-class classifier
namespace meta
{
namespace classify
{
template <>
std::unique_ptr<classifier>
    make_classifier<my_classifier>(
        const cpptoml::toml_group& config,
        std::shared_ptr<index::forward_index> idx);
}
}

// if you have a multi-class classifier that requires an
// inverted_index
namespace meta
{
namespace classify
{
template <>
std::unique_ptr<classifier>
    make_multi_indexclassifier<my_classifier>(
        const cpptoml::toml_group& config,
        std::shared_ptr<index::forward_index> idx,
        std::shared_ptr<index::inverted_index> inv_idx);
}
}

// if you have a binary classifier
namespace meta
{
namespace classify
{
template <>
std::unique_ptr<classifier>
    make_binary_classifier<my_binary_classifier>(
        const cpptoml::toml_group& config,
        std::shared_ptr<index::forward_index> idx,
        class_label positive_label,
        class_label negative_label);
}
}
```

---

[Home]({{site.baseurl}}/)
&nbsp; | &nbsp;
[Overview]({{site.baseurl}}/overview-tutorial.html)
&nbsp; | &nbsp;
[Analyzers and Filters]({{site.baseurl}}/analyzers-filters-tutorial.html)
&nbsp; | &nbsp;
[Indexes]({{site.baseurl}}/index-tutorial.html)
&nbsp; | &nbsp;
[Classifiers]({{site.baseurl}}/classify-tutorial.html)
&nbsp; | &nbsp;
[Topic Models]({{site.baseurl}}/topic-models-tutorial.html)
