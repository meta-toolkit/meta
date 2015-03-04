---
title: Parsing
layout: page
category: tut
order: 8
---

Many applications that deal with natural language desire constituency (or
syntax) trees that describe the individual phrases in sentences and how
they are hierarchically combined. [Wikipedia has a nice example][wp-parse].

MeTA provides a constituency parser in the `meta::parser` namespace. This
is a very efficient parser that is based on the shift-reduce parsing
algorithm. For more information on the algorithm itself, refer to the
following papers:

- [Fast and Accurate Shift-Reduce Constituency Parsing][faasrcp]
- [Transition-Based Parsing of the Chinese Treebank using a Global
    Discriminative Model][trans-based]

# Using the parser
First, you will need to download a parser model file from the [releases
page][releases] on the Github repository. MeTA currently supplies two
trained shift-reduce parser models:

- A greedy, best-first parser (i.e. a beam size of 1)
- A beam-search parser with a maximum beam size of 4

Choosing between the two models is a time/performance tradeoff. The greedy
parser is signifigantly faster than the beam-search parser, but achieves
slightly lower accuracy than the beam-search parser. It should be noted,
however, that either model is likely much faster than traditional
PCFG-based parsers due to their lower algorithmic complexity during
parsing.

In order to use the parser, you will need to have a `[parser]`
configuration group in `config.toml` that looks like the following:

{% highlight toml %}
[parser]
prefix = "path/to/parser/model"
{% endhighlight %}

You will also likely want a part-of-speech tagging model, as the parser
operates over pre-tagged sequences (it does not assign POS tags on its
own). You should refer to the [part-of-speech tagging tutorial][pos-tut]
for more information on the part-of-speech tagging models that MeTA
provides.

## Interactive parsing
You can interactively parse sentences using the `profile` tool. See the
[`profile` tutorial][profile-tut] for a walkthrough of that demo
application.

## As an analyzer
The shift-reduce parser is now what powers MeTA's [structural tree
feature][stf] extraction. In order to use the parser's supplied
`tree_analyzer` in your analyzer pipeline, you will need to add an analyzer
group to `config.toml` that looks like the following:

{% highlight toml %}
[[analyzers]]
method = "tree"
filter = [{type = "icu-tokenizer"}, {type = "ptb-normalizer"}]
features = ["skel", "subtree"]
tagger = "path/to/greedy-tagger/model"
parser = "path/to/sr-parser/model"
{% endhighlight %}

There are a few things to note here. First, you **must** provide a path to
a trained parser **as well as** a trained **greedy tagger**. The analyzer
currently is not configured to use the CRF tagger (though this may be added
in the future: patches welcome!).

You may modify the filter chain if you would like, but we strongly
recommend sticking with he above setup as it is designed to match the
original Penn Treebank tokenization format that the supplied models were
trained on.

The supported features that can be specified in the `features` array are
"branch", "depth", "semi-skel", "skel", "subtree", and "tag". These
correspond to the feature types given in the structural tree feature paper
linked above.

While you could use multiple `[[analyzers]]` groups for each individual
tree feature you would like to compute, it is much preferable to list all
of the features you want in the `features` key to avoid re-parsing
sentences for each feature.

## Programmatically
To use the shift-reduce parser inside your own program, your code might
look like this:

{% highlight cpp %}
// load the models
meta::sequence::perceptron tagger{"path/to/perceptron/model/folder"};
meta::parser::sr_parser parser{"path/to/sr-parser/model/folder"};

meta::sequence::sequence seq;
// - code for loading/creating the sequence here -

// tag the sequence
tagger.tag(seq);

// parse the tagged sequence
auto tree = parser.parse(seq);

// print the parse tree in an indented, human-readable format
tree.pretty_print(std::cout);

// print the parse tree in a collapsed, machine-readable format
std::ofstream outfile{"my_output_file.trees"};
outfile << tree;
{% endhighlight %}

Have a look at the
[API documentation for the `meta::parser::sr_parser` class][parser] for
more information.

# Training the parser

In order to train your own models using our provided programs, you will
need to have a copy of the Penn Treebank (v3) extracted into your data
prefix. (see [the overview tutorial][overview-tut]). Your folder structure
should look like the following:

~~~
prefix
|---- penn-treebank
      |---- treebank-3
            |---- parsed
                  |---- mrg
                        |---- wsj
                              |---- 00
                              |---- 01
                              ...
                              |---- 24
~~~

You will need to expand your `[parser]` configuration group in
`config.toml` to look like the following:

{% highlight toml %}
[parser]
prefix = "parser"
treebank = "penn-treebank" # relative to data prefix
corpus = "wsj"
section-size = 99
# these are the standard training/development/testing splits for parsing
train-sections = [2, 21]
dev-sections = [22, 22]
test-sections = [23, 23]
{% endhighlight %}

You can also add the following additional keys to configure training
behavior:

- `train-threads`: controls the number of training threads used (defaults
   to the number of processors on the machine)
- `train-algorithm`: controls the algorithm used for training. This can be
   one of the following:
   * "early-termination" (default): trains a greedy parser (beam size = 1)
   * "beam-search": trains a parser using beam search
- `beam-size`: controls the size of the beam used during training. This
   option is ignored if the algorithm is "early-termination"

There are more options that can be tweaked, but the remaining options must
be done programmatically. See the [API documentation for the
`meta::parser::sr_parser` class][parser] for more information (in
particular, the `training_options` struct). If you want to try different
options, you can use the code provided in
`src/parser/tools/parser_train.cpp` as a starting point.

The parser may take several hours to train, depending on your training
parameters. The greedy parser ("early-termination") trains much faster than
the beam search parser.

# Testing the parser

If you follow the instructions above, you should be able to test a parser
model by running the following:
{% highlight bash %}
./parser-test config.toml tree-output
{% endhighlight %}

The application will run over the test set configured in `config.toml` and
reports many of the [evalb metrics][evalb-doc]. However, it is **always**
best practice to run the output trees against the [standard evalb program
itself][evalb], which is why the trees are also output to a file.

The greedy parser obtains the following results:

~~~
Labeled Recall:    86.9455
Labeled Precision: 86.6949
Labeled F1:        86.82
~~~

The beam search parser (with a beam size of 4) obtains the following
results:

~~~
Labeled Recall:    88.2171
Labeled Precision: 88.0778
Labeled F1:        88.1474
~~~

[wp-parse]: http://en.wikipedia.org/wiki/Parse_tree#Constituency-based_parse_trees
[faasrcp]: http://people.sutd.edu.sg/~yue_zhang/pub/acl13.muhua.pdf
[trans-based]: http://www.aclweb.org/anthology/W09-3825
[releases]: https://github.com/meta-toolkit/meta/releases
[pos-tut]: {{ site.baseurl }}/pos-tagging-tutorial.html
[profile-tut]: {{ site.baseurl }}/profile-tutorial.html
[stf]: http://web.engr.illinois.edu/~massung1/files/icsc-2013.pdf
[parser]: {{ site.baseurl }}/doxygen/classmeta_1_1parser_1_1sr__parser.html
[overview-tut]: {{ site.baseurl }}/overview-tutorial.html
[evalb-doc]: {{ site.basurl }}/doxygen/classmeta_1_1parser_1_1evalb.html
[evalb]: http://nlp.cs.nyu.edu/evalb/
