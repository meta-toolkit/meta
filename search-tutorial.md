---
layout: default
title: Tutorial - Search
---

## Search in MeTA

At the heart of MeTA lies its indexes. Every piece of data that MeTA's
algorithms operate on must first reside in an index. In this tutorial, we'll
walk you through how to create your own `inverted_index` and how to use your
`inverted_index` to create a search engine.

## Initially setting up the config file

By default, there are several fields at the top of your config file. These are
general settings that deal with paths:

{% highlight toml %}
prefix = "/home/sean/projects/meta-data/"
libsvm-modules = "../deps/libsvm-modules/"
punctuation = "../data/sentence-boundaries/sentence-punctuation.txt"
stop-words = "../data/lemur-stopwords.txt"
start-exceptions = "../data/sentence-boundaries/sentence-start-exceptions.txt"
end-exceptions = "../data/sentence-boundaries/sentence-end-exceptions.txt"
function-words = "../data/function-words.txt"
{% endhighlight %}

I have set `prefix` to be where I store my datasets. You can leave the other
paths as they are; they'll work by default.

Next, we have a section for the corpus (20newsgroups) and its analyzer.

{% highlight toml %}
corpus-type = "line-corpus"
dataset = "20newsgroups"
#list = "20news"
forward-index = "20newsgroups-fwd"
inverted-index = "20newsgroups-inv"
encoding = "latin-1"

[[analyzers]]
method = "ngram-word"
ngram = 1
filter = "default-chain"
{% endhighlight %}

`corpus-type` tells MeTA whether our dataset is a `line-corpus` (one doc per
line in a large file) or a `file-corpus` (each doc is an individual file). If
you're using a `file-corpus`, you need to specify the `list` parameter
(commented out above) so MeTA can find the file `20news-full-corpus.txt` in the
dataset directory.

If the executable needs to search the index, we can add the following to say
which ranker to use:

{% highlight toml %}
[ranker]
method = "bm25"
{% endhighlight %}

For all the premade ranking functions, you can additionally specify any of the
ranking function parameters:

{% highlight toml %}
[ranker]
method = "bm25"
k1 = 1.2
b = 0.75
k3 = 500
{% endhighlight %}

For a list of MeTA's ranking functions, see the
[index::ranker](http://meta-toolkit.github.io/meta/doxygen/classmeta_1_1index_1_1ranker.html)
documentation.

## Using the example apps

Now that we know how to set up our config file, let's experiment with some
indexes. **It's possible to index and search a corpus without writing any
code!** In the root directory of MeTA, there are several example apps that deal
with creating indexes for search engines or information retrieval tasks:

 - `index.cpp`: indexes a corpus
 - `interactive-search.cpp`: searches for documents based on user input
 - `query-runner.cpp`: runs a list of queries specified by a file

Let's go through these one by one:

### Index

{% highlight bash %}
./index config.toml
{% endhighlight %}

You can then see it being indexed. After it's done, some stats are printed out,
such as number of documents, average document length, and number of unique
terms. If you run it again, it will just print out the stats since it detects
that the index has already been created.

### Interactive Search

{% highlight bash %}
./interactive-search config.toml
{% endhighlight %}

Running this will first create an index if one does not exist. Then, it will
prompt the user to type a query. The current analyzer is run on the text query
to turn it into a document, and then that document is used to search. The top
results are returned.

### Query Runner

For this app, we need to add a path to the query file.

{% highlight toml %}
querypath = "./"
{% endhighlight %}

The query file contains one query per line, and they are run on the current
index. You quickly make your own query file in the current directory if you
don't have one. It must be named `dataset-queries.txt`, where in our example
`dataset` is `20newsgroups`.

{% highlight bash %}
./query-runner config.toml
{% endhighlight %}

The top 10 documents for each query are then displayed along with the time
taken to run all the queries.

## Writing code

Now we'll look at some examples of how to create and search an index. All this
code can be found in the demo apps as well.

### Creating an index

This is the simplest way to make an inverted index:

{% highlight cpp %}
auto idx = index::make_index<index::inverted_index>(argv[1]);
{% endhighlight %}

This assumes that `argv[1]` is the path to the TOML config file, which is
standard MeTA practice.

Warning: this index does not have a cache! A cache is used when running queries
on the index. The cache saves frequently used `postings_data` objects so we
don't need to do a disk seek for every term while scoring.

Here are two different ways to specify a cache:

{% highlight cpp %}
// Create an inverted index using a DBLRU cache. The arguments forwarded to
//  make_index are the config file for the index and any parameters for the
//  cache. In this case, we set the maximum hash table size for the
//  dblru_cache to be 10000.
auto idx = index::make_index<index::dblru_inverted_index>(argv[1], 10000);

// Create an inverted index using a splay cache. The arguments forwarded
//  to make_index are the config file for the index and any parameters
//  for the cache. In this case, we set the maximum number of nodes in
//  the splay_cache to be 10000.
auto idx = index::make_index<index::splay_inverted_index>(argv[1], 10000);
{% endhighlight %}

### Searching an index

Now that we have an index, let's search it! First, we need to create a `ranker`:

{% highlight cpp %}
auto config = cpptoml::parse_file(argv[1]);
auto group = config.get_table("ranker");
if (!group)
    throw std::runtime_error{"\"ranker\" group needed in config file!"};
auto ranker = index::make_ranker(*group);
{% endhighlight %}

We can also manually specify our `ranker` instead of reading from the config
file:

{% highlight cpp %}
index::okapi_bm25 ranker{1.2, 0.75, 500.0};
{% endhighlight %}

We don't have to specify the arguments for the ranking parameters, but we did
anyway.

Finally, we are able to search our index with our ranker. Let's assume we used
`make_ranker` to create the ranker, so the `ranker` object is a
`std::unique_ptr` to the actual object.

{% highlight cpp %}
corpus::document query{"[doc path]", doc_id{0}};
query.content("my query text");

auto ranking = ranker->score(*idx, query);
{% endhighlight %}

`ranking` is a sorted vector of `pair`s of (`doc_id`,`double`). By default, the
top 10 documents are returned. To return a different number, just add a third
parameter:

{% highlight cpp %}
auto ranking = ranker->score(*idx, query, 25);
{% endhighlight %}

Iterate through `ranking` and display your results! Check out the
[index::ir_eval](http://meta-toolkit.github.io/meta/doxygen/classmeta_1_1index_1_1ir__eval.html)
class if you have relevance judgements for your queries.

## Writing your own ranker

Perhaps you'd like to experiment and write your own ranking function. Any
rankers you write should subclass `index::ranker` and implement the pure
virtual function `score_one(const score_data& sd)`. `score_data` is a struct
that contains useful information to ranking functions. Read up on it
[here](http://meta-toolkit.github.io/meta/doxygen/structmeta_1_1index_1_1score__data.html).

If you would like to be able to create your ranker by specifying it in a
configuration file, you will need to provide a public static string id member
that specifies the text that identifies your ranker class, and register it with
the toolkit somewhere in main() like this:

{% highlight cpp %}
meta::index::register_ranker<my_ranker>();
{% endhighlight %}

If you need to read parameters from the config group, you should specialize the
`make_ranker()` function as follows:

{% highlight cpp %}
namespace meta
{
namespace index
{
template <>
std::unique_ptr<ranker> make_ranker<my_ranker>(const cpptoml::table& config)
{
    // read config file
    // set default params if config options not found (for example)
}
}
}
{% endhighlight %}

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
