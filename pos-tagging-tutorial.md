---
title: Part of Speech Tagging
layout: page
category: tut
order: 7
---

MeTA also provides models that can be used for
[part-of-speech tagging][pos-tagging]. These models, at the moment, are
designed for tagging English text, but they should be able to be trained
for any language desired once appropriate feature extractors are defined.

To **use** these models, you should download a tagger model file from the
[releases page][releases] on the Github repository. MeTA currently has two
different POS-tagger models available:

- A linear-chain conditional random field ([`meta::sequence::crf`][crf])
- An averaged Perceptron, greedy tagger
    ([`meta::sequence::perceptron`][perceptron])

# Using Taggers

## Using the CRF
First, extract your model files into a directory. You should modify your
`config.toml` to contain a `[crf]` group like so:

{% highlight toml %}
[crf]
prefix = "path/to/crf/model/folder"
{% endhighlight %}

where `prefix` has been set to the folder that contains the model files you
extracted.

### Interactive tagging
You can interactively tag sentences using the provided `pos-tag` tool.

{% highlight bash %}
./pos-tag config.toml
{% endhighlight %}

This application will load the CRF model, and then proceed to tag sentences
typed in at the prompt. You can stop tagging by simply inputting a blank
line.

### As an analyzer
The CRF model can also be used as an analyzer during index creation to
create features based on n-grams of part-of-speech tags. To do so, you
would need to add an analyzer group to your configuration that looks like
the following:

{% highlight toml %}
[[analyzers]]
type = "ngram-pos"
ngram = 2 # the order n-gram you want to generate
filter = [{type = "icu-tokenizer"}, {type = "ptb-normalizer"}]
crf-prefix = "path/to/crf/model/folder"
{% endhighlight %}

You can alter the filter chain if you would like, but we strongly recommend
sticking with the above setup as it is designed to match the original Penn
Treebank tokenization format that the supplied model is trained on.

### Programmatically
To use the CRF inside your own program, your code might look like this:

{% highlight cpp %}
// load the model
meta::sequence::crf model{"path/to/crf/model/folder"};

// create a tagger
auto tagger = crf.make_tagger();

// load the sequence analyzer (for feature generation)
auto analyzer = meta::sequence::default_pos_analyzer();
analyzer.load(*crf_prefix);

meta::sequence::sequence seq;
// - code for loading/creating the sequence here -

// tag a sequence
const auto& ana = analyzer; // access the analyzer via const ref
                            // so that no new feature ids are generated
ana.analyze(seq);
tagger.tag(seq);

// print the tagged sequence
for (const auto& obs : seq)
    std::cout << obs.symbol() << "_" << analyzer.tag(obs.label()) << " ";
std::cout << "\n";
{% endhighlight %}

Have a look at the
[API documentation for the `meta::sequence::crf` class][crf] for more
information.

## Using the greedy tagger (Perceptron)
First, extract your model files into a directory. You should modify your
`config.toml` to contain a `[sequence]` group like so:

{% highlight toml %}
[sequence]
prefix = "path/to/perceptron/model/folder/"
{% endhighlight %}

where `prefix` has been set to the folder that contains the model files you
extracted.

### Interactive tagging
The `pos-tag` tool doesn't currently use this tagger (patches welcome!),
but you can still interactively tag sentences using the `profile` tool. See
the [`profile` tutorial][profile-tut] for a walkthrough of that demo
application.

### Programmatically
To use the greedy Perceptron-based tagger inside your own program, your
code might look like this:

{% highlight cpp %}
// load the model
meta::sequence::perceptron tagger{"path/to/perceptron/model/folder"};

meta::sequence::sequence seq;
// - code for loading/creating the sequence here -

// tag a sequence
tagger.tag(seq);

// print the tagged sequence
for (const auto& obs : seq)
    std::cout << obs.symbol() << "_" << obs.tag() << " ";
std::cout << "\n";
{% endhighlight %}

This API is a bit simpler than that of the CRF. For more information, you
can check the
[API documentation for the `meta::sequence::perceptron` class][perceptron].

# Training Taggers

In order to train your own models using our provided training programs, you
will need to have a copy of the Penn Treebank (v2) extracted into
your data prefix (see [the overview tutorial][overview-tut]). Your folder
structure should look like the following:

~~~
prefix
|---- penn-treebank
      |---- treebank-2
            |---- tagged
                  |---- wsj
                        |---- 00
                        |---- 01
                        ...
                        |---- 24
~~~

## Training a CRF
To train your own CRF model from the Penn Treebank data, you should be able
to use the provided `crf-train` executable. You will first need to adjust
your `[crf]` group in your `config.toml` to look something like this:

{% highlight toml %}
[crf]
prefix = "desired/crf/model/location"
treebank = "penn-treebank" # relative to data prefix
corpus = "wsj"
section-size = 99
# these are the standard training/development/testing splits for POS tagging
train-sections = [0, 18]
dev-sections = [19, 21]
test-sections = [22, 24]
{% endhighlight %}

You should now be able to run the training procedure:
{% highlight bash %}
./crf-train config.toml
{% endhighlight %}

This will train a CRF model using the default training options. For more
information on the options available, please see the [API documentation for
the `meta::sequence::crf` class][crf] (in particular, the parameters
struct). If you would like to try different options, you can use the code
provided in `src/sequence/crf/tools/crf_train.cpp` as a starting point. You
will need to change the call to `crf.train()` to use a non-default
`parameters` struct.

The model will take several hours to train. Its termination is based on
convergence of the loss function.

## Training a greedy Perceptron-based tagger
To train your own greedy tagger model from the Penn Treebank data, you
should be able to use the provided `greedy-tagger-train` executable. You
will need to first adjust your `[sequence]` group in your `config.toml` to
look something like this (very similar to the above):

{% highlight toml %}
[sequence]
prefix = "desired/perceptron/model/location"
treebank = "penn-treebank" # relative to data prefix
corpus = "wsj"
section-size = 99
# these are the standard training/development/testing splits for POS tagging
train-sections = [0, 18]
dev-sections = [19, 21]
test-sections = [22, 24]
{% endhighlight %}

You should now be able to run the training procedure:
{% highlight bash %}
./greedy-tagger-train config.toml
{% endhighlight %}

This will train the averaged Perceptron model using the default training
options. The termination criteria is simply a maximum iteration count,
which defaults to 5 as of the time of writing. This means that the greedy
tagger is **signifigantly** faster to train than its corresponding CRF
model. In practice, the two achieve nearly the same accuracy with our
default settings (the CRF being just slightly better).

If you want to adjust the number of training iterations, you can use the
code provided in `src/sequence/tools/greedy_tagger_train.cpp` as a starting
point. You will need to change the call to `tagger.train()` to use a
non-default `training_options` struct.

# Testing Taggers
If you follow the instructions above for the tagger type you wish to test,
you should be able to test them with their corresponding testing
executables. For the CRF, you would use
{% highlight bash %}
./crf-test config.toml
{% endhighlight %}
and for the greedy Perceptron-based tagger, you would use
{% highlight bash %}
./greedy-tagger-test config.toml
{% endhighlight %}

Both will run over the testing section defined in `config.toml` and report
precision, recall, and F1 score for each class, as well as the overall
token-level accuracy. The current CRF model achieves 97% accuracy, and the
greedy Perceptron model achieves 96.9% accuracy.

[pos-tagging]: http://en.wikipedia.org/wiki/Part-of-speech_tagging
[releases]: https://github.com/meta-toolkit/meta/releases
[crf]: {{ site.baseurl }}/doxygen/classmeta_1_1sequence_1_1crf.html
[perceptron]: {{ site.baseurl }}/doxygen/classmeta_1_1sequence_1_1perceptron.html
[profile-tut]: {{ site.baseurl }}/profile-tutorial.html
[overview-tut]: {{ site.baseurl }}/overview-tutorial.html
