/**
 * @file profile.cpp
 * @author Sean Massung
 */

#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>
#include "meta/analyzers/analyzer.h"
#include "meta/analyzers/tokenizers/icu_tokenizer.h"
#include "meta/analyzers/filters/all.h"
#include "meta/analyzers/ngram/ngram_word_analyzer.h"
#include "meta/corpus/document.h"
#include "cpptoml.h"
#include "meta/io/filesystem.h"
#include "meta/parser/sr_parser.h"
#include "meta/sequence/io/ptb_parser.h"
#include "meta/sequence/perceptron.h"
#include "meta/sequence/sequence.h"
#include "meta/util/shim.h"

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
    std::cerr << "\t--parse\tcreate grammatical parse trees from file content"
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
 * @param stream Token stream to read from
 * @param in_name Input filename
 * @param out_name Output filename
 */
template <class Stream>
void write_file(Stream& stream, const std::string& in_name,
                const std::string& out_name)
{
    std::ofstream outfile{out_name};
    stream->set_content(filesystem::file_text(in_name));
    while (*stream)
    {
        auto next = stream->next();
        if (next == "<s>" || next == " ")
            continue;
        else if (next == "</s>")
            outfile << std::endl;
        else
            outfile << next << " ";
    }
}

/**
 * Performs stemming on a text file.
 * @param file The input file
 * @param config Configuration settings
 */
void stem(const std::string& file, const cpptoml::table&)
{
    std::cout << "Running stemming algorithm" << std::endl;

    using namespace meta::analyzers;
    std::unique_ptr<token_stream> stream
        = make_unique<tokenizers::icu_tokenizer>();
    stream = make_unique<filters::lowercase_filter>(std::move(stream));
    stream = make_unique<filters::porter2_filter>(std::move(stream));
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
void stop(const std::string& file, const cpptoml::table& config)
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
void pos(const std::string& file, const cpptoml::table& config, bool replace)
{
    std::cout << "Running POS-tagging with replace = " << std::boolalpha
              << replace << std::endl;

    auto seq_grp = config.get_table("sequence");
    if (!seq_grp)
    {
        std::cerr << "[sequence] group needed in config file" << std::endl;
        return;
    }

    auto prefix = seq_grp->get_as<std::string>("prefix");
    if (!prefix)
    {
        std::cerr << "[sequence] group needs a prefix key" << std::endl;
        return;
    }

    std::cout << "Loading tagging model" << std::endl;
    sequence::perceptron tagger{*prefix};

    // construct the token filter chain
    std::unique_ptr<analyzers::token_stream> stream
        = make_unique<analyzers::tokenizers::icu_tokenizer>();
    stream = make_unique<analyzers::filters::ptb_normalizer>(std::move(stream));

    stream->set_content(filesystem::file_text(file));

    // tag each sentence in the file
    // and write its output to the output file
    auto out_name
        = no_ext(file) + (replace ? ".pos-replace.txt" : ".pos-tagged.txt");
    std::ofstream outfile{out_name};
    sequence::sequence seq;
    while (*stream)
    {
        auto token = stream->next();
        if (token == "<s>")
        {
            seq = {};
        }
        else if (token == "</s>")
        {
            tagger.tag(seq);
            for (const auto& obs : seq)
            {
                if (replace)
                    outfile << obs.tag() << " ";
                else
                    outfile << obs.symbol() << "_" << obs.tag() << " ";
            }
            outfile << std::endl;
        }
        else
        {
            seq.add_symbol(sequence::symbol_t{token});
        }
    }

    std::cout << " -> file saved as " << out_name << std::endl;
}

/**
 * Parses all sentences in a text file.
 */
void parse(const std::string& file, const cpptoml::table& config)
{
    std::cout << "Running parser" << std::endl;

    auto seq_grp = config.get_table("sequence");
    if (!seq_grp)
    {
        std::cerr << "[sequence] group needed in config file" << std::endl;
        return;
    }

    auto prefix = seq_grp->get_as<std::string>("prefix");
    if (!prefix)
    {
        std::cerr << "[sequence] group needs a prefix key" << std::endl;
        return;
    }

    auto parser_grp = config.get_table("parser");
    if (!parser_grp)
    {
        std::cerr << "[parser] group needed in config file" << std::endl;
        return;
    }

    auto parser_prefix = parser_grp->get_as<std::string>("prefix");
    if (!parser_prefix)
    {
        std::cerr << "[parser] group needs a prefix key" << std::endl;
        return;
    }

    std::cout << "Loading tagging model" << std::endl;
    // load POS-tagging model
    sequence::perceptron tagger{*prefix};

    std::cout << "Loading parser model" << std::endl;
    // load parser model
    parser::sr_parser parser{*parser_prefix};

    // construct the token filter chain
    std::unique_ptr<analyzers::token_stream> stream
        = make_unique<analyzers::tokenizers::icu_tokenizer>();
    stream = make_unique<analyzers::filters::ptb_normalizer>(std::move(stream));

    stream->set_content(filesystem::file_text(file));

    // parse each sentence in the file
    // and write its output to the output file
    auto out_name = no_ext(file) + ".parsed.txt";
    std::ofstream outfile{out_name};
    sequence::sequence seq;
    while (*stream)
    {
        auto token = stream->next();
        if (token == "<s>")
        {
            seq = {};
        }
        else if (token == "</s>")
        {
            tagger.tag(seq);
            parser.parse(seq).pretty_print(outfile);
        }
        else
        {
            seq.add_symbol(sequence::symbol_t{token});
        }
    }

    std::cout << " -> file saved as " << out_name << std::endl;
}

/**
 * Performs frequency analysis on a text file.
 * @param file The input file
 * @param config Configuration settings
 * @param n The n-gram value to use in tokenization
 */
void freq(const std::string& file, cpptoml::table& config, int64_t n)
{
    std::cout << "Running frequency analysis on " << n << "-grams" << std::endl;
    using namespace meta::analyzers;

    // make sure we have the right n-gram
    auto anas = config.get_table_array("analyzers");
    auto local = *anas->begin();
    local->erase("ngram");
    local->insert("ngram", n);
    auto ana = make_analyzer<ngram_word_analyzer>(config, *local);

    corpus::document doc;
    doc.content(filesystem::file_text(file));
    auto counts = ana->analyze<uint64_t>(doc);

    using pair_t = std::pair<std::string, uint64_t>;
    std::vector<pair_t> sorted(counts.begin(), counts.end());
    std::sort(sorted.begin(), sorted.end(), [](const pair_t& a, const pair_t& b)
              {
                  return a.second > b.second;
              });

    auto out_name = no_ext(file) + ".freq." + std::to_string(n) + ".txt";
    std::ofstream outfile{out_name};
    for (auto& token : sorted)
        outfile << token.first << " " << token.second << std::endl;

    std::cout << " -> file saved as " << out_name << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 4)
        return print_usage(argv[0]);

    auto config = cpptoml::parse_file(argv[1]);
    std::string file = argv[2];
    if (!filesystem::file_exists(file))
    {
        std::cerr << "File does not exist" << std::endl;
        return 1;
    }
    std::unordered_set<std::string> args{argv + 3, argv + argc};
    bool all = args.find("--all") != args.end();

    if (all || args.find("--stem") != args.end())
        stem(file, *config);
    if (all || args.find("--stop") != args.end())
        stop(file, *config);
    if (all || args.find("--pos") != args.end())
        pos(file, *config, false);
    if (all || args.find("--pos-replace") != args.end())
        pos(file, *config, true);
    if (all || args.find("--parse") != args.end())
        parse(file, *config);
    if (all || args.find("--freq-unigram") != args.end())
        freq(file, *config, 1);
    if (all || args.find("--freq-bigram") != args.end())
        freq(file, *config, 2);
    if (all || args.find("--freq-trigram") != args.end())
        freq(file, *config, 3);
}
