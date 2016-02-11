/**
 * @file corpus_factory.cpp
 * @author Chase Geigle
 */

#include "meta/corpus/all.h"
#include "meta/corpus/corpus_factory.h"

namespace meta
{
namespace corpus
{

template <class Corpus>
void corpus_factory::reg()
{
    add(Corpus::id, make_corpus<Corpus>);
}

corpus_factory::corpus_factory()
{
    // built-in corpora
    reg<file_corpus>();
    reg<line_corpus>();
    reg<gz_corpus>();
    reg<libsvm_corpus>();
}

std::unique_ptr<corpus> make_corpus(const cpptoml::table& config)
{
    auto corp = config.get_as<std::string>("corpus");
    if (!corp)
        throw corpus_exception{"corpus missing from configuration file"};

    auto prefix = config.get_as<std::string>("prefix");
    if (!prefix)
        throw corpus_exception{"prefix missing from configuration file"};

    auto dataset = config.get_as<std::string>("dataset");
    if (!dataset)
        throw corpus_exception{"dataset missing from configuration file"};

    auto corpus_filename = *prefix + "/" + *dataset + "/" + *corp;
    if (!filesystem::file_exists(corpus_filename))
        throw corpus_exception{"corpus configuration file (" + corpus_filename
                               + ") not present"};

    auto corpus_config = cpptoml::parse_file(corpus_filename);
    auto type = corpus_config->get_as<std::string>("type");
    if (!type)
        throw corpus_exception{"type missing from corpus configuration file"};

    auto result = corpus_factory::get().create(*type, *prefix, *dataset,
                                               *corpus_config);

    result->set_metadata_parser({*prefix + "/" + *dataset + "/metadata.dat",
                                 metadata_schema(*corpus_config)});

    auto store_full_text
        = corpus_config->get_as<bool>("store-full-text").value_or(false);
    result->set_store_full_text(store_full_text);

    return result;
}
}
}
