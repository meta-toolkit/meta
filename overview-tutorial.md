---
layout: default
title: Tutorial - MeTA System Overview
---

## MeTA's goal

What is MeTA's goal?

## How does MeTA store data?

Explain how everything is stored in an index

## Corpus input formats

How can you use your corpus with MeTA?

There are currently two corpus input formats:

* `line_corpus`
* `file_corpus`

Also mention about `.pos` and `.tree`, perhaps link to
`meta-stanford-preprocesor`.

## Datasets

There are several public datasets that we've converted to one of the MeTA input
formats. They are available...

## Analyzers and filters

Basic idea of analyzers and filters, more info in [Analyzers and
Filters]({{site.baseurl}}/analyzers-filters-tutorial.html) tutorial

## Unit tests

We're using [ctest](http://www.cmake.org/cmake/help/v2.8.8/ctest.html), which
is configured to run all the tests available in the `unit-test.cpp` file.
You may run

```bash
CTEST_OUTPUT_ON_FAILURE=1 ctest
```

to execute the unit tests while displaying output from failed tests.

The file `unit-test.cpp`, takes various tests as parameters. For example,

```bash
./unit-test inverted-index
```

or

```bash
./unit-test all
```

Please note that you must have a valid configuration file (config.toml) in the
project root, since many unit tests create input based on paths stored there.

You won't normally have to worry about running `./unit-test` yourself since we
have ctest set up. However, since ctest creates a new process for each test, it
is hard to debug. If you'd like to debug with tools such as valgrind or gdb,
running `./unit-test` manually is a better option.

## Bugs

On GitHub, create an issue and make sure to label it as a bug. Please be as
detailed as possible. Inlcuding a code snippet would be very helpful if
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

Who develops MeTA?

## Users

Who uses MeTA?

## License

MeTA is released under the [MIT License](http://opensource.org/licenses/MIT). It
is a free, permissive license, meaning it is allowed to be included in
proprietary software.

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
