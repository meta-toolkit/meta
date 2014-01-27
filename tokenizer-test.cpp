/**
 * @file tokenizer-test.cpp
 * Simply tokenizes a series of documents from a corpus.
 */

#include <iostream>

#include "cpptoml.h"
#include "tokenizers/all.h"
#include "corpus/all.h"
#include "util/printing.h"
#include "util/time.h"

using namespace meta;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cerr << "Usage:\t" << argv[0] << " configFile" << std::endl;
        return 1;
    }

    auto config = cpptoml::parse_file(argv[1]);
    auto corpus = corpus::corpus::load(argv[1]);
    auto tok = tokenizers::tokenizer::load(config);
    const uint64_t total = 2000;

    auto elapsed = common::time<std::chrono::seconds>([&](){
        std::string progress = " Tokenizing ";
        while(corpus->has_next())
        {
            auto doc = corpus->next();
            if(doc.id() >= total)
                break;
            printing::show_progress(doc.id(), total, 100, progress);
            tok->tokenize(doc);
            //std::cout << doc.content() << std::endl;
        }
        printing::end_progress(progress);
    });

    std::cerr << "Tokenizing took " << elapsed.count() << "s" << std::endl;
}
