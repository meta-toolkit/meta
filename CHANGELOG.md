# [v2.4.2][2.4.2]
## Bug Fixes
- Properly shuffle documents when doing an even-split classification test
- Make forward indexer listen to `indexer-num-threads` config option.
- Use correct number of threads when deciding block sizes for
    `parallel_for`
- Add workaround to `filesystem::remove_all` for Windows systems to avoid
    spurious failures caused by virus scanners keeping files open after we
    deleted them
- Fix invalid memory access in `gzstreambuf::underflow`

# [v2.4.1][2.4.1]
## Bug fixes
- Eliminate excess warnings on Darwin about double preprocessor definitions
- Fix issue finding `config.h` when used as a sub-project via
    add_subdirectory()

# [v2.4.0][2.4.0]
## New features
- Add a minimal perfect hashing implementation for `language_model`, and unify
  the querying interface with the existing language model.
- Add a CMake `install()` command to install MeTA as a library (issue #143). For
  example, once the library is installed, users can do:

    ```
    find_package(MeTA 2.4 REQUIRED)

    add_executable(my-program src/my_program.cpp)
    target_link_libraries(my-program meta-index) # or whatever other libs you
    need from MeTA
    ```
- Feature selection functionality added to `multiclass_dataset` and
  `binary_dataset` and views (issues #111, #149 and PR #150 thanks to @siddshuk).

  ```cpp
    auto selector = features::make_selector(*config, training_vw);
    uint64_t total_features_selected = 20;
    selector->select(total_features_selected);
    auto filtered_dset = features::filter_dataset(dset, *selector);
  ```
- Users can now, similar to `hash_append`, declare standalone functions in the
  same scope as their type called `packed_read` and `packed_write` which will be
  called by `io::packed::read` and `io::packed::write`, respectively, via
  argument-dependent lookup.

## Bug fixes
- Fix edge-case bug in the succinct data structures
- Fix off-by-one error in `lm::diff`

## Enhancements
- Added functionality to the `meta::hashing` library: `hash_append` overload for
  `std::vector`, manually-seeded hash function
- Further isolate ICU in MeTA to allow CMake to `install()`
- Updates to EWS (UIUC) build guide
- Add `std::vector` operations to `io::packed`
- Consolidated all variants of chunk iterators into one template
- Add MeTA's citation to the README!

# [v2.3.0][2.3.0]
## New features
- Forward and inverted indexes are now stored in one directory. **To make
    use of your existing indexes, you will need to move their
    directories.** For example, a configuration that used to look like the
    following

    ```toml
    dataset = "20newsgroups"
    corpus = "line.toml"
    forward-index = "20news-fwd"
    inverted-index = "20news-inv"
    ```

    will now look like the following

    ```toml
    dataset = "20newsgroups"
    corpus = "line.toml"
    index = "20news-index"
    ```

    and your folder structure should now look like

    ```
    20news-index
    ├── fwd
    └── inv
    ```

    You can do this by simply moving the old folders around like so:

    ```bash
    mkdir 20news-index
    mv 20news-fwd 20news-index/fwd
    mv 20news-inv 20news-index/inv
    ```
- `stats::multinomial` now can report the number of unique event types
    counted (`unique_events()`)
- `std::vector` can now be hashed via `hash_append`.

## Bug fixes
- Fix rounding bug in language model-based rankers. This bug caused
    severely degraded performance for these rankers with short queries. The
    unit tests have been improved to prevent such a regression in the
    future.

## Enhancements
- The bundled ICU version has been bumped to ICU 57.1.
- MeTA will now attempt to build its own version of ICU on Windows if it
    fails to find a suitable ICU installed.
- CI support for GCC 6.x was added for all three major platforms.
- CI support also uses a fixed version of LLVM/libc++ instead of trunk.

# [v2.2.0][2.2.0]
## New features
- Parallelized versions of PageRank and Personalized PageRank have been
    added. A demo is available in `wiki-page-rank`; see the website for
    more information on obtaining the required data.
- Add a disk-based streaming minimal perfect hash function library. A
    sub-component of this is a small memory-mapped succinct data structure
    library for answering rank/select queries on bit vectors.
- Much of our CMake magic has been moved into a separate project included
    as a submodule: https://github.com/meta-toolkit/meta-cmake, which can
    now be used in other projects to simplify initial build system
    configuration.

## Bug fixes
- Fix parameter settings in language model rankers not being range checked
    (issue #134).
- Fix incorrect incoming edge insertion in `directed_graph::add_edge()`.
- Fix `find_first_of` and `find_last_of` in `util::string_view`.

## Enhancements
- `forward_index` now knows how to tokenize a document down to a
    `feature_vector`, provided it was generated with a non-LIBSVM analyzer.
- Allow loading of an existing index where its corpus is no longer
    available.
- Data is no longer shuffled in `batch_train`. Shuffling the data
    causes horrible access patterns in the postings file, so the data
    should instead shuffled before indexing.
- `util::array_view`s can now be constructed as empty.
- `util::multiway_merge` has been made more generic. You can now specify
    both the comparison function and merging criteria as parameters, which
    default to `operator<` and `operator==`, respectively.
- A simple utility classes `io::mifstream` and `io::mofstream` have been
    added for places where a moveable `ifstream` or `ofstream` is desired
    as a workaround for older standard libraries lacking these move
    constructors.
- The number of indexing threads can be controlled via the configuration
    key `indexer-num-threads` (which defaults to the number of threads on
    the system), and the number of threads allowed to concurrently write to
    disk can be controlled via `indexer-max-writers` (which defaults to 8).

# [v2.1.0][2.1.0]
## New features
- Add the [GloVe algorithm](http://www-nlp.stanford.edu/pubs/glove.pdf) for
  training word embeddings and a library class `word_embeddings` for loading and
  querying trained embeddings. To facilitate returning word embeddings, a simple
  `util::array_view` class was added.
- Add simple vector math library (and move `fastapprox` into the `math`
  namespace).

## Bug fixes
- Fix `probe_map::extract()` for `inline_key_value_storage` type; old
  implementation forgot to delete all sentinel values before returning the
  vector.
- Fix incorrect definition of `l1norm()` in `sgd_model`.
- Fix `gmap` calculation where 0 average precision was ignored
- Fix progress output in `multiway_merge`.

## Enhancements
- Improve performance of `printing::progress`. Before, `progress::operator()` in
  tight loops could dramatically hurt performance, particularly due to frequent
  calls to `std::chrono::steady_clock::now()`. Now, `progress::operator()`
  simply sets an atomic iteration counter and a background thread periodically
  wakes to update the progress output.
- Allow full text storage in index as metadata field. If `store-full-text =
  true` (default false) in the corpus config, the string metadata field
  "content" will be added. This is to simplify the creation of full text
  metadata: the user doesn't have to duplicate their dataset in `metadata.dat`,
  and `metadata.dat` will still be somewhat human-readable without large strings
  of full text added.
- Allow `make_index` to take a user-supplied corpus object.

## Miscellaneous
- ZLIB is now a required dependency.
- Switch to just using the standalone `./unit-test` instead of `ctest`. There
  aren't really many advantages for us to using CTest at this point with the new
  unit test framework, so just use our unit test executable.

# [v2.0.1][2.0.1]
## Bug fixes
- Fix issue where `metadata_parser` would not consume spaces in string
    metadata fields. Thanks to @hopsalot on the forum for the bug report!
- Fix build issue on OS X with Xcode 6.4 and `clang` related to their
    shipped version of `string_view` lacking a const `to_string()` method

## Enhancements
- The `./profile` executable ensures that the file exists before operating on
  it. Thanks to @domarps for the PR!
- Add a generic `util::multiway_merge` algorithm for performing the
    merge-step of an external memory merge sort.
- Build with the following Xcode versions on Travis CI:
  * Xcode 6.1 and OS X 10.9 (as before)
  * Xcode 6.4 and OS X 10.10 (new)
  * Xcode 7.1.1 and OS X 10.10 (new)
  * Xcode 7.2 and OS X 10.11 (new)

# [v2.0.0][2.0.0]
## New features and major changes

### Indexing
- Index format rewrite: both inverted and forward indices now use the same
    compressed postings format, and intermediate chunks are now also
    compressed on-the-fly. There is now a built in tool to dump any forward
    index to libsvm format (as this is not the on-disk format for that type
    of index anymore).
- Metadata support: indices can now store arbitrary metadata associated
    with individual documents with string, integer, unsigned integer, and
    floating point values
- Corpus configuration is now stored within the corpus directory itself,
    allowing for corpora to be distributed with their proper configurations
    rather than having to bake this into the main configuration file
- RAM limits can be set for the indexing process via the configuration
    file. These are **approximate** and based on heuristics, so you should
    always set these to lower than available RAM.
- Forward indices can now be created directly instead of forcing the
    creation of an inverted index first

### Tokenization and Analysis
- ICU will be built and statically linked if the system provided library is
    too old on both OS X and Linux platforms. MeTA now will specify an
    exact version of ICU that should be used per release for consistency.
    That version is 56.1 as of this release.
- Analyzers have been modified to support both integral and floating point
    values via the use of the `featurizer` object passed to `tokenize()`
- Documents no longer store any count information during the analysis
    process

### Ranking
- Postings lists can now be read in a streaming fashion rather than all at
    once via `postings_stream`
- Ranking is now performed using a document-at-a-time scheme
- Ranking functions now use fast approximate math from
    [fastapprox][fastapprox]
- Rank correlation measures have been added to the evaluation library

### Language Model
- Rewrite of the language model library which can load models from the
  [.arpa][arpa] format
- [SyntacticDiff][syndiff] implementation for comparative text mining, which may
  include grammatical error correction, summarization, or feature generation

### Machine Learning
- A feature selection library for selecting features for machine learning
    using chi square, information gain, correlation coefficient, and odds
    ratio has been added
- The API for the machine learning algorithms has been changed to use
    `dataset` classes; these are separate from the index classes and
    represent data that is memory-resident
- Support for regression has been added (currently only via SGD)
- The SGD algorithm has been improved to use a normalized adaptive gradient
    method which should make it less sensitive to feature scaling
- The SGD algorithm now supports (approximate) L1 regularization via a
    cumulative penalty approach
- The libsvm modules are now also built using CMake

### Miscellaneous
- Packed binary I/O functions allow for writing integers/floating point
    values in a compressed format that can be efficiently decoded. This
    should be used for most binary I/O that needs to be performed in the
    toolkit unless there is a specific reason not to.
- An interactive demo application has been added for the shift-reduce
    constituency parser
- A `string_view` class is provided in the `meta::util` namespace to be
    used for non-owning references to strings. This will use
    `std::experimental::string_view` if available and our own
    implementation if not
- `meta::util::optional` will resolve to `std::experimental::optional` if
    it is available
- Support for jemalloc has been added to the build system. We **strongly**
    recommend installing and linking against jemalloc for improved indexing
    performance.
- A tool has been added to print out the top *k* terms in a corpus
- A new library for hashing has been added in namespace `meta::hashing`.
    This includes a generic framework for writing hash functions that are
    randomly keyed as well as (insertion only) probing-based hash sets/maps
    with configurable resizing and probing strategies
- A utility class `fixed_heap` has been added for places where a fixed size
    set of maximal/minimal values should be maintained in constant space
- The filesystem management routines have been converted to use STLsoft in
    the event that the filesystem library in
    `std::experimental::filesystem` is not available
- Building MeTA on Windows is now officially supported via MSYS2 and
    MinGW-w64, and continuious integration now builds it on every commit in
    this environment
- A small support library for things related to random number generation
    has been added in `meta::random`
- Sparse vectors now support `operator+` and `operator-`
- An STL container compatible allocator `aligned_allocator<T, Alignment>`
    has been added that can over-align data (useful for performance in some
    situations)
- Bandit is now used for the unit tests, and these have been substantially
    improved upon
- `io::parser` deprecated and removed; most uses simply converted to
  `std::fstream`
- `binary_file_{reader,writer}` deprecated and removed;
  `io::packed` or `io::{read,write}_binary` should be used instead

## Bug fixes
- knn classifier now only requests the top *k* when performing classification
- An issue where uncompressed model files would not be found if using a
    zlib-enabled build (#101)

## Enhancements
- Travis CI integration has been switched to their container
    infrastructure, and it now builds with OS X with Clang in addition to
    Linux with Clang and GCC
- Appveyor CI for Windows builds alongside Travis
- Indexing speeds are dramatically faster (thanks to many changes both in
    the in-memory posting chunks as well as optimizations in the
    tokenization process)
- If no build type is specified, MeTA will be built in Release mode
- The cpptoml dependency version has been bumped, allowing the use of
    things like `value_or` for cleaner code
- The identifiers library has been dramatically simplified

[syndiff]: http://web.engr.illinois.edu/~massung1/files/bigdata-2015.pdf
[fastapprox]: https://code.google.com/p/fastapprox/
[arpa]: http://www.speech.sri.com/projects/srilm/manpages/ngram-format.5.html

# [v1.3.8][1.3.8]
## Bug fixes
- Fix issue with `confusion_matrix` where precision and recall values were
  swapped. Thanks to @husseinhazimeh for finding this!

## Enhancements
- Better unit tests for `confusion_matrix`
- Add functions to `confusion_matrix` to directly access precision, recall, and
    F1 score
- Create a `predicted_label` opaque identifier to emphasize `class_labels` that
    are output from some model (and thus shouldn't be interchangeable)

# [v1.3.7][1.3.7]
## Bug fixes
- Fix inconsistent behavior of `utf::segmenter` (and thus `icu_tokenizer`) for
    different locales. Thanks @CanoeFZH and @tng-konrad for helping debug
    this!

## Enhancements
- Allow for specifying the language and country for locale generation in
    setting up `utf::segmenter` (and thus `icu_tokenizer`)
- Allow for suppression of `<s>` and `</s>` tags within `icu_tokenizer`,
    mostly useful for information retrieval experiments with unigram words.
    Thanks @husseinhazimeh for the suggestion!
- Add a `default-unigram-chain` filter chain preset which is suitable for
    information retrieval experiments using unigram words. Thanks
    @husseinhazimeh for the suggestion!

# [v1.3.6][1.3.6]
## Bug fixes
- Fix potential off-by-one when calculating the number of documents in a
    `line_corpus` when its files do not end in a newline

## Enhancements
- Change `score_data` to support floating-point weights on query terms

# [v1.3.5][1.3.5]
## Bug fixes
- Fix missing support for sequence/parser analyzers in the classify tools

# [v1.3.4][1.3.4]
## New features
- Support building with biicode
- Add Vagrantfile for virtual machine configuration
- Add Dockerfile for Docker support

## Enhancements
- Improve `ir_eval` unit tests

## Bug fixes
- Fix `ir_eval::ndcg` incorrect log base and addition instead of subtraction in
    IDCG calculation
- Fix `ir_eval::avg_p` incorrect early termination

# [v1.3.3][1.3.3]
## Bug fixes
- Fix issues with system-defined integer widths in binary model files
    (mainly impacted the greedy tagger and parser); please re-download any
    parser model files you may have had before
- Fix bug where parser model directory is not created if a non-standard
    prefix is used (anything other than "parser")

## Enhancements
- Silence inconsistent missing overrides warning on clang >= 3.6

# [v1.3.2][1.3.2]
## Bug fixes
- fix potentially incorrect generation of vocabulary map files on 32-bit
    systems (this appears to have only impacted non-default block sizes)

# [v1.3.1][1.3.1]
## Bug fixes
- fix calculation of average precision in `ir_eval` (the denominator was
    incorrect)
- specify that labels are required for the `file_corpus` document list; this
    allows spaces in the path to each document

# [v1.3][1.3]
## New features
- additions to the graph library:
    * myopic search
    * BFS
    * preferential attachment graph generation model (supports node
        attractiveness from different distributions)
    * betweenness centrality
    * eigenvector centrality
- added a new natural language parsing library:
    * parse tree library (visitor-based)
    * shift-reduce constituency parser for generating phrase structure
        trees
    * reimplementation of evalb metrics for evaluating parsers
    * new filter for Penn Treebank-style normalization
- added a greedy averaged Perceptron-based tagger
- demo application for various basic text processing (profile)
- basic iostreams that support gzip compression (if compiled with ZLib
    support)
- added iteration method for `stats::multinomial` seen events
- added expected value and entropy functions to `stats` namespace
- added `linear_model`: a generic multiclass classifier storage class
- added `gz_corpus`: a compressed version of `line_corpus`
- added macros for generating type safe identifiers with user defined
    literal suffixes
- added a persistent stack data structure to `meta::util`

## Enhancements
- added operator== for `util::optional<T>`
- better CMake support for building the libsvm modules
- better CMake support for downloading unit-test data
- improved setup guide in README (for OS X, Ubuntu, Arch, and EWS/ENGRIT)
- tree analyzers refactored to use the new parser library (removes
    dependency on outside toolkits for generating tree files)
- analyzers that are not part of the "core" have been moved into their
    respective folders (so `ngram_pos_analyzer` is in `src/sequence`,
    `tree_analyzer` is in `src/parser`)
- `make_index` now checks if the files exist before loading an index, and
    if they are missing creates a new one (as opposed to just throwing an
    exception on a nonexistent file)
- cpptoml upgraded to support TOML v0.4.0
- enable extra warnings (-Wextra) for clang++ and g++

## Bug fixes
- fix `sequence_analyzer::analyze() const` when applied to untagged
    sequences (was throwing when it shouldn't)
- ensure that the inverted index object is destroyed first before
    uninverting occurs in the creation of a `forward_idnex`
- fix bug where `icu_tokenizer` would output spaces as tokens
- fix bugs where index objects were not destroyed before trying to delete
    their files in the unit tests
- fix bug in `sparse_vector::find()` where it would return a non-end
    iterator when asked to find an element that does not exist

# [v1.2][1.2]
## New features
- demo application for CRF-based POS tagging
- `nearest_centroid` classifier
- basic statistics library for representing relevant probability
    distributions
- `sparse_vector` utility class

## Enhancements
- `ngram_pos_analyzer` now uses the CRf internally (see issue #46)
- `knn` classifier new supports weighted knn
- `filesystem::copy_file()` no longer hangs without progress reporting with
    large files
- CMake build system now includes `INTERFACE` targets (better inclusion as
    a subproject in external projects)
- MeTA can now (optionally) be built with C++14 support

## Bug fixes
- `language_model_ranker` scoring function corrected (see issue #50)
- `naive_bayes` classifier scoring corrected
- several incorrect instances of `numeric_limits<double>::min()` replaced
    with the intended `numeric_limits<double>::lowest()`
- fix compilation with versions of ICU < 4.4

# [v1.1][1.1]
## Changes
- sequence analyzer and CRF implementation
- basic language model
- basic directed and undirected graphs
- restructure CMakeLists

# [v1.0][1.0]
- Initial release.

[unreleased]: https://github.com/meta-toolkit/meta/compare/v2.4.2...develop
[2.4.2]: https://github.com/meta-toolkit/meta/compare/v2.4.1...v2.4.2
[2.4.1]: https://github.com/meta-toolkit/meta/compare/v2.4.0...v2.4.1
[2.4.0]: https://github.com/meta-toolkit/meta/compare/v2.3.0...v2.4.0
[2.3.0]: https://github.com/meta-toolkit/meta/compare/v2.2.0...v2.3.0
[2.2.0]: https://github.com/meta-toolkit/meta/compare/v2.1.0...v2.2.0
[2.1.0]: https://github.com/meta-toolkit/meta/compare/v2.0.1...v2.1.0
[2.0.1]: https://github.com/meta-toolkit/meta/compare/v2.0.0...v2.0.1
[2.0.0]: https://github.com/meta-toolkit/meta/compare/v1.3.8...v2.0.0
[1.3.8]: https://github.com/meta-toolkit/meta/compare/v1.3.7...v1.3.8
[1.3.7]: https://github.com/meta-toolkit/meta/compare/v1.3.6...v1.3.7
[1.3.6]: https://github.com/meta-toolkit/meta/compare/v1.3.5...v1.3.6
[1.3.5]: https://github.com/meta-toolkit/meta/compare/v1.3.4...v1.3.5
[1.3.4]: https://github.com/meta-toolkit/meta/compare/v1.3.3...v1.3.4
[1.3.3]: https://github.com/meta-toolkit/meta/compare/v1.3.2...v1.3.3
[1.3.2]: https://github.com/meta-toolkit/meta/compare/v1.3.1...v1.3.2
[1.3.1]: https://github.com/meta-toolkit/meta/compare/v1.3...v1.3.1
[1.3]: https://github.com/meta-toolkit/meta/compare/v1.2...v1.3
[1.2]: https://github.com/meta-toolkit/meta/compare/v1.1...v1.2
[1.1]: https://github.com/meta-toolkit/meta/compare/v1.0...v1.1
[1.0]: https://github.com/meta-toolkit/meta/compare/01aff7e0bddfaba997141d96ef7a371b3221e0ee...v1.0
