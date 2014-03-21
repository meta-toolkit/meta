---
layout: default
title: Tutorial - Online Learning
---

## Online Learning in MeTA

In many cases, the datasets we deal with in large machine learning
problems are too large to fit into the working-set memory of a modest
computer. Fortunately, MeTA's use of indexes for storing its data make it
very capable of handling these cases when coupled with a classifier that
supports online learning (such as `sgd` coupled with any of the
appropriate loss functions, or a `one_vs_all` or `one_vs_one` ensemble of
these classifiers). In this tutorial, we'll explore performing online
learning of an `sgd`-trained support vector machine (SVM) on a dataset from
the LIBSVM dataset website,
[rcv1.binary](http://www.csie.ntu.edu.tw/~cjlin/libsvmtools/datasets/binary.html#rcv1.binary).
(This dataset is not actually large enough to require an online learning
algorithm, but it is large enough to demonstrate the value in this
approach, and one can easily see how the method can be extended to
datasets that truly do require an online-learning approach.)

Download the training and testing sets and place them in your data
directory under a folder called `rcv1`. Then, construct an `rcv1.dat` from
the following command (or equivalent):

{% highlight bash %}
bzcat rcv1_test.binary.bz2 rcv1_train.binary.bz2 > rcv1.dat
{% endhighlight %}

(Note: we will be using the given test set as the training set and the
given training set as the test set: this is mostly just so that the
training set is sizeable enough to appreciate the memory usage of the
toolkit during training: reversing the splits gives us approximately 1.2GB
of training data to process).

The approach we will take here is to run the `sgd` training algorithm
several times on "mini-batches" of the data. The dataset has a total of
677399 training examples, and if we loaded these all into memory it would
take nearly a gigabyte of working memory. Instead, we will load the data
into memory in chunks of <= 50000 documents and train on each chunk
individually.

Below is the relevant portion of our `config.toml` for this example:

{% highlight toml %}
corpus-type = "line-corpus"
dataset = "rcv1"
forward-index = "rcv1-fwd"
inverted-index = "rcv1-inv"

# the size of the mini-batches: this may need to be set empirically based
# on the amount of memory available on the target system
batch-size = 50000
# the document-id where the test set begins
test-start = 677399

[[analyzers]]
method = "libsvm"

[classifier]
method = "one-vs-all"
    [classifier.base]
    method = "sgd"
    loss = "hinge"
    prefix = "sgd-model"
{% endhighlight %}

Now, we can run the provided application with `./online-classify
config.toml`, see results that look something like the following:

<div>
<code>
<pre>
$ ./online-classify config.toml
1395363665: [info]     Loading index from disk: rcv1-fwd
(/home/chase/projects/meta/src/index/forward_index.cpp:137)
 > Counting lines in file: [================================] 100% ETA 00:00:00
Training batch 14/14

           -1       1
         ------------------
      -1 | <strong>0.966</strong>    0.0344
       1 | 0.0274   <strong>0.973</strong>

------------------------------------------------
<strong>Class</strong>       <strong>F1 Score</strong>    <strong>Precision</strong>   <strong>Recall</strong>
------------------------------------------------
-1          0.968       0.966       0.97
1           0.97        0.973       0.968
------------------------------------------------
<strong>Total</strong>       <strong>0.969</strong>       <strong>0.969</strong>       <strong>0.969</strong>
------------------------------------------------
20242 predictions attempted, overall accuracy: 0.969
Took 119s
</pre>
</code>
</div>

At the time of this tutorial's writing, this classification process peaks at
about 95MB---a far cry away from the 1.2GB training set! This general process
should be able to be extended to work with any dataset that cannot be fit into
memory, provided an appropriate `batch-size` is set in the configuration file.

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
