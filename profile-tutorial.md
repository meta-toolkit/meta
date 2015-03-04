---
title: Basic Text Analysis
subtitle: A whirlwind tour of some features of MeTA
layout: page
category: tut
order: 2
---

MeTA focuses on providing rich functionality for text mining applications.
This tutorial will use the provided `profile` application to demonstrate
some of the text analysis features that are built in to MeTA. It is not
completely comprehensive, but it should give you an idea of some of the
analysis you can do using MeTA as a framework.

We **strongly** encourage you to read the source for this demo application,
which is located in `src/tools/profile.cpp` and its corresponding
`src/tools/CMakeLists.txt`. This file is heavily commented and should serve
as an example starting point for using MeTA programmatically.

# Setup
Obtain a text file for analysis. You can either create this manually, copy
some text from the Internet, or even use this very tutorial as your text
source. Save it in your `build` directory as `doc.txt`. We will use this
file as raw data, processing it through several different tools provided by
MeTA.

# Shallow text analysis
To start, we will analyze `doc.txt` from a "shallow" point of view. We will
look at our document as just a set of sentences composed of individual
tokens.

## Stop word removal
When viewing text in this mode, it's often beneficial to reduce the number
of unique tokens we are considering, especially if they do not contribute
to the meaning of the text. Words like *the*, *of*, and *to* do very little
in telling us about the content of `doc.txt`, so it's often safe to remove
them and still keep the gist of the document intact. Removing these
uninformative, frequently occurring words can dramatically reduce the
amount of text we need to process.

MeTA provides a default list of these "stop words" in
`data/lemur-stopwords.txt`, which have been taken from the [Lemur
project][lemur].

Remove the stop words from your document by running the following command:

{% highlight bash %}
./profile config.toml doc.txt --stop
{% endhighlight %}

The output should be saved to the file `doc.stops.txt`. To see how this was
done, refer to the `stop()` function in `src/tools/profile.cpp`. This is
utilizing MeTA's [tokenizers and filters][ana-tut] to create a pipeline that
outputs processed tokens.

## Stemming
In many applications (and particularly for retrieval) it is beneficial to
reduce words down to their base forms. For example, if someone searches for
*running*, it is reasonable to return results that match *run* or *runs* in
addition to documents that match *running* directly. MeTA ships with an
implementation of the [Porter2 Stemmer for English][porter2], which is used
in the default analysis pipeline. Some example stems are listed below.

~~~
{run, runs, running} -> run
{argue, argued, argues, arguing} -> argu
{lies, lying, lie} -> lie
~~~

Reduce all words down to their stems in your document by running the
following command:

{% highlight bash %}
./profile config.toml doc.txt --stem
{% endhighlight %}

The output should be saved to the file `doc.stems.txt`. To see how this was
done, refer to the `stem()` function in `src/tools/profile.cpp`. This is
another example of utilizing MeTA's [tokenizers and filters][ana-tut].

## Frequency analysis
One convenient way of representing documents is the so called "bag of
words" representation. In this view, documents are represented simply as
vectors of word counts. All position information is thrown away. While this
seems naive at first, it is actually a fairly effective representation for
a number of applications, including document retrieval, topic modeling, and
document classification.

You can calculate the bag of words representation for your document by
running the following commands:

{% highlight bash %}
# a bag of individual words
./profile config.toml doc.txt --freq-unigram

# a bag of two consecutive word sequences
./profile config.toml doc.txt --freq-bigram

# a bag of three consecutive word sequences
./profile config.toml doc.txt --freq-trigram
{% endhighlight %}

The output should be saved to the files `doc.freq.n.txt` where "n" is 1, 2,
or 3 depending on the n-gram choice (unigram, bigram, or trigram) specified
in the flags above. To see how this was done, refer to the `freq()` method
in `src/tools/profile.cpp`. This is an example of using an
[analyzer][ana-tut] to reduce a document into a sparse feature vector
(here, each feature is the name of an n-gram).

# Natural language processing (NLP)
Depending on the application, it may be useful to extract features that are
more linguistically motivated than just the tokens themselves. MeTA
supports a few common tasks used when trying to get a more *deep*
understanding of a document: part-of-speech tagging and phrase structure
(or constituency) parsing.

## Part-of-speech tagging
Every word in the English language can be assigned at least one class (or
tag) that indicates its "part(s)-of-speech". For example, the word *the* is
a determiner (DT), and the word *token* is a singular noun (NN). Here is [a
list of commonly used part-of-speech tags][ptb-pos-tags] for English as
defined by the Penn Treebank project.

The task of taking a sequence of words and assigning each word a
part-of-speech (POS) tag is referred to as "part-of-speech tagging". MeTA
supports part of speech tagging with a few different models. Let's use one
of them to POS tag our `doc.txt`.

First, visit the [releases page][releases], click the latest version,
and download the model files for the "greedy part-of-speech tagger".
Extract these into a folder and take note of the path to that folder.

Ensure that you have a section in `config.toml` that looks like the
following:

{% highlight toml %}
[sequence]
prefix = "path/to/your/tagger/folder/"
{% endhighlight %}

Now, you should be able to POS-tag your `doc.txt` by running the following
command:

{% highlight bash %}
./profile config.toml doc.txt --pos
{% endhighlight %}

The output should be written to `doc.pos-tagged.txt`. To see how this was
done, refer to the `pos()` method in `src/tools/profile.cpp`. This is an
example of using a [greedy Perceptron-based tagger][pos-tut] for POS
tagging.

## Parsing
Sometimes it is beneficial to get a deeper understanding of the structure
of a sentence beyond just assigning each word a part-of-speech. Language is
recursive and hierarchical---one task in NLP is to determine the phrase
structure tree that describes how the parts of a sentence connect with one
another. [Wikipedia has a good example.][wp-parse]

MeTA has support for inferring the phrase structure tree that was used to
generate a sentence. Let's use this capability to transform each sentence
in our `doc.txt` into a separate phrase structure tree.

First, visit the [releases page][releases], click on the latest version,
and download the model files for the "greedy shift-reduce constituency
parser". Extract these into a folder and take note of the path to that
folder.

Ensure that you have a section in `config.toml` that looks like the
following:

{% highlight toml %}
[parser]
prefix = "path/to/your/parser/folder/"
{% endhighlight %}

Since the parser relies on having POS-tagged sentences, ensure that you've
done the part-of-speech tagging section of this tutorial as well.

Parse the sentences in `doc.txt` by running the following command:

{% highlight bash %}
./profile config.toml doc.txt --parse
{% endhighlight %}

The output should be written to `doc.parsed.txt`. To see how this was done,
refer to the `parse()` method in `src/tools/profile.cpp`. This is an
example of using an efficient [shift-reduce constituency parser][parse-tut]
for deriving phrase structure trees.

[lemur]: http://www.lemurproject.org/
[ana-tut]: {{ site.baseurl }}/analyzers-filters-tutorial.html
[porter2]: http://snowball.tartarus.org/algorithms/english/stemmer.html
[ptb-pos-tags]: https://www.ling.upenn.edu/courses/Fall_2003/ling001/penn_treebank_pos.html
[releases]: https://github.com/meta-toolkit/meta/releases
[pos-tut]: {{ site.baseurl }}/pos-tagging-tutorial.html
[wp-parse]: http://en.wikipedia.org/wiki/Parse_tree#Constituency-based_parse_trees
[parse-tut]: {{ site.baseurl }}/parsing-tutorial.html
