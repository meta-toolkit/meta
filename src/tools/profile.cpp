/**
 * @file profile.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
#include "cpptoml.h"
#include "util/shim.h"
#include "analyzers/analyzer.h"
#include "analyzers/tokenizers/icu_tokenizer.h"
#include "analyzers/filters/lowercase_filter.h"
#include "analyzers/filters/porter2_stemmer.h"
#include "analyzers/filters/empty_sentence_filter.h"
#include "analyzers/filters/list_filter.h"
#include "analyzers/ngram/ngram_word_analyzer.h"
#include "corpus/document.h"
#include "sequence/crf/crf.h"
#include "sequence/crf/tagger.h"
#include "sequence/io/ptb_parser.h"
#include "sequence/sequence.h"
#include "sequence/crf/tagger.h"

using namespace meta;

/**
 * Prints help for this executable.
 * @param prog The name of the current executable
 * @return the exit code for this program
 */
int print_usage(const std::string& prog)
{
    std::cerr << std::endl;
    std::cerr << "Usage: " << prog << " config.toml file.txt [OPTION]"
              << std::endl;
    std::cerr << "where [OPTION] is one or more of:" << std::endl;
    std::cerr << "\t--stem\tperform stemming on each word" << std::endl;
    std::cerr << "\t--stop\tremove stopwords" << std::endl;
    std::cerr << "\t--pos\tannotate words with POS tags" << std::endl;
    std::cerr << "\t--pos-replace\treplace words with their POS tags"
              << std::endl;
    std::cerr << "\t--freq-unigram\tsort and count unigram words" << std::endl;
    std::cerr << "\t--freq-bigram\tsort and count bigram words" << std::endl;
    std::cerr << "\t--freq-trigram\tsort and count trigram words" << std::endl;
    std::cerr << "\t--all\trun all options" << std::endl;
    std::cerr << std::endl;
    return 1;
}

/**
 * @param file The filename to modify
 * @return the base filename without an extension
 */
std::string no_ext(const std::string& file)
{
    auto idx = file.find_last_of('.');
    return file.substr(0, idx);
}

/**
 * @param in_name The filename to read
 * @return string content from the given file
 */
std::string file_text(const std::string& in_name)
{
    std::ifstream infile{in_name};
    std::ostringstream buf;
    buf << infile.rdbuf();
    return buf.str();
}

/**
 * @param stream Token stream to read from
 * @param in_name Input filename
 * @param out_name Output filename
 */
template <class Stream>
void write_file(Stream& stream, const std::string& in_name,
                const std::string& out_name)
{
    std::ofstream outfile{out_name};
    stream->set_content(file_text(in_name));
    while (*stream)
    {
        auto next = stream->next();
        if (next == "<s>" || next == " ")
            continue;
        outfile << next;
        if (next == "</s>")
            outfile << std::endl;
    }
}

/**
 * Performs stemming on a text file.
 * @param file The input file
 * @param config Configuration settings
 */
void stem(const std::string& file, const cpptoml::toml_group& config)
{
    std::cout << "Running stemming algorithm" << std::endl;

    using namespace meta::analyzers;
    std::unique_ptr<token_stream> stream
        = make_unique<tokenizers::icu_tokenizer>();
    stream = make_unique<filters::lowercase_filter>(std::move(stream));
    stream = make_unique<filters::porter2_stemmer>(std::move(stream));
    stream = make_unique<filters::empty_sentence_filter>(std::move(stream));

    auto out_name = no_ext(file) + ".stems.txt";
    write_file(stream, file, out_name);
    std::cout << " -> file saved as " << out_name << std::endl;
}

/**
 * Performs stopword removal on a text file.
 * @param file The input file
 * @param config Configuration settings
 */
void stop(const std::string& file, const cpptoml::toml_group& config)
{
    std::cout << "Running stopword removal" << std::endl;

    using namespace meta::analyzers;
    auto stopwords = config.get_as<std::string>("stop-words");
    std::unique_ptr<token_stream> stream
        = make_unique<tokenizers::icu_tokenizer>();
    stream = make_unique<filters::lowercase_filter>(std::move(stream));
    stream = make_unique<filters::list_filter>(std::move(stream), *stopwords);
    stream = make_unique<filters::empty_sentence_filter>(std::move(stream));

    auto out_name = no_ext(file) + ".stops.txt";
    write_file(stream, file, out_name);
    std::cout << " -> file saved as " << out_name << std::endl;
}

/**
 * Performs part-of-speech tagging on a text file.
 * @param file The input file
 * @param config Configuration settings
 * @param replace Whether or not to replace words with their POS tags
 */
void pos(const std::string& file, const cpptoml::toml_group& config,
         bool replace)
{
    std::cout << "Running POS-tagging with replace = " << std::boolalpha
              << replace << std::endl;

    using namespace meta::sequence;

    // load POS-tagging model
    auto crf_group = config.get_group("crf");
    auto prefix = crf_group->get_as<std::string>("prefix");
    crf crf_model{*prefix};
    const sequence_analyzer analyzer = default_pos_analyzer(*prefix);
    auto tagger = crf_model.make_tagger();

    // read file into a sequence
    std::unique_ptr<analyzers::token_stream> stream
        = make_unique<analyzers::tokenizers::icu_tokenizer>();
    stream->set_content(file_text(file));
    sequence::sequence seq;
    while (*stream)
    {
        auto token = stream->next();
        if (token == " ")
            continue;
        seq.add_observation({symbol_t{token}, tag_t{"[UNK]"}});
    }

    // annotate sequence with POS tags
    analyzer.analyze(seq);
    tagger.tag(seq);

    // write output to file
    auto out_name = no_ext(file)
                    + (replace ? ".pos-replace.txt" : ".pos-tagged.txt");
    std::ofstream outfile{out_name};
    for (auto& obs : seq)
    {
        if (obs.symbol() == symbol_t{"<s>"})
            continue;
        if (obs.symbol() == symbol_t{"</s>"})
        {
            outfile << std::endl;
            continue;
        }
        if (replace)
            outfile << analyzer.tag(obs.label()) << " ";
        else
            outfile << obs.symbol() << "_" << analyzer.tag(obs.label()) << " ";
    }

    std::cout << " -> file saved as " << out_name << std::endl;
}

/**
 * Performs frequency analysis on a text file.
 * @param file The input file
 * @param config Configuration settings
 * @param n The n-gram value to use in tokenization
 */
void freq(const std::string& file, const cpptoml::toml_group& config,
          uint16_t n)
{
    std::cout << "Running frequency analysis on " << n << "-grams" << std::endl;

    std::unique_ptr<analyzers::token_stream> stream
        = make_unique<analyzers::tokenizers::icu_tokenizer>();
    analyzers::ngram_word_analyzer ana{n, std::move(stream)};

    corpus::document doc;
    doc.content(file_text(file));
    ana.tokenize(doc);

    using pair_t = std::pair<std::string, double>;
    std::vector<pair_t> sorted(doc.counts().begin(), doc.counts().end());
    std::sort(sorted.begin(), sorted.end(), [](const pair_t& a, const pair_t& b)
    {
        return a.second > b.second;
    });

    auto out_name = no_ext(file) + ".freq." + std::to_string(n) + ".txt";
    std::ofstream outfile{out_name};
    for (auto& token : sorted)
        outfile << token.first << ": " << token.second << std::endl;

    std::cout << " -> file saved as " << out_name << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 4)
        return print_usage(argv[0]);

    auto config = cpptoml::parse_file(argv[1]);
    std::string file = argv[2];
    std::unordered_set<std::string> args{argv + 3, argv + argc};
    bool all = args.find("--all") != args.end();

    if (all || args.find("--stem") != args.end())
        stem(file, config);
    if (all || args.find("--stop") != args.end())
        stop(file, config);
    if (all || args.find("--pos") != args.end())
        pos(file, config, false);
    if (all || args.find("--pos-replace") != args.end())
        pos(file, config, true);
    if (all || args.find("--freq-unigram") != args.end())
        freq(file, config, 1);
    if (all || args.find("--freq-bigram") != args.end())
        freq(file, config, 2);
    if (all || args.find("--freq-trigram") != args.end())
        freq(file, config, 3);
}
