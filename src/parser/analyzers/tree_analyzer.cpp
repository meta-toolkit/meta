/**
 * @file tree_analyzer.cpp
 * @author Chase Geigle
 */

#include "cpptoml.h"
#include "meta/analyzers/token_stream.h"
#include "meta/parser/analyzers/tree_analyzer.h"
#include "meta/parser/analyzers/featurizers/featurizer_factory.h"
#include "meta/utf/segmenter.h"

namespace meta
{
namespace analyzers
{

const util::string_view tree_analyzer::id = "tree";

tree_analyzer::tree_analyzer(std::unique_ptr<token_stream> stream,
                             const std::string& tagger_prefix,
                             const std::string& parser_prefix)
    : featurizers_{std::make_shared<tree_featurizer_list>()},
      stream_{std::move(stream)},
      tagger_{std::make_shared<sequence::perceptron>(tagger_prefix)},
      parser_{std::make_shared<parser::sr_parser>(parser_prefix)}
{
    // nothing
}

tree_analyzer::tree_analyzer(const tree_analyzer& other)
    : featurizers_{other.featurizers_},
      stream_{other.stream_->clone()},
      tagger_{other.tagger_},
      parser_{other.parser_}
{
    // nothing
}

void tree_analyzer::add(std::unique_ptr<const tree_featurizer> featurizer)
{
    featurizers_->emplace_back(std::move(featurizer));
}

void tree_analyzer::tokenize(const corpus::document& doc, featurizer& counts)
{
    stream_->set_content(get_content(doc));

    sequence::sequence seq;
    while (*stream_)
    {
        auto next = stream_->next();

        if (next == "<s>")
        {
            seq = {};
        }
        else if (next == "</s>")
        {
            tagger_->tag(seq);
            auto tree = parser_->parse(seq);
            for (const auto& featurizer : *featurizers_)
                featurizer->tree_tokenize(tree, counts);
        }
        else
        {
            seq.add_symbol(sequence::symbol_t{next});
        }
    }
}

template <>
std::unique_ptr<analyzer>
make_analyzer<tree_analyzer>(const cpptoml::table& global,
                             const cpptoml::table& config)
{
    auto tagger_prefix = config.get_as<std::string>("tagger");
    if (!tagger_prefix)
        throw analyzer_exception{"tree analyzer requires a tagger directory"};

    auto parser_prefix = config.get_as<std::string>("parser");
    if (!parser_prefix)
        throw analyzer_exception{"tree analyzer requires a parser directory"};

    auto feat_arr = config.get_array("features");
    if (!feat_arr)
        throw analyzer_exception{
            "tree analyzer needs an array of features to generate"};

    auto filts = load_filters(global, config);
    auto ana = make_unique<tree_analyzer>(std::move(filts), *tagger_prefix,
                                          *parser_prefix);

    for (const auto& feat : feat_arr->array_of<std::string>())
        ana->add(featurizer_factory::get().create(feat->get()));

    return std::move(ana);
}
}

namespace parser
{
void register_analyzers()
{
    using namespace analyzers;
    register_analyzer<tree_analyzer>();
}
}
}
