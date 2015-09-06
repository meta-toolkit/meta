/**
 * @file tree_analyzer.cpp
 * @author Chase Geigle
 */

#include "cpptoml.h"
#include "analyzers/token_stream.h"
#include "parser/analyzers/tree_analyzer.h"
#include "parser/analyzers/featurizers/featurizer_factory.h"
#include "utf/segmenter.h"

namespace meta
{
namespace analyzers
{

template <class T>
const util::string_view tree_analyzer<T>::id = "tree";

template <class T>
tree_analyzer<T>::tree_analyzer(std::unique_ptr<token_stream> stream,
                                const std::string& tagger_prefix,
                                const std::string& parser_prefix)
    : featurizers_{std::make_shared<tree_featurizer_list>()},
      stream_{std::move(stream)},
      tagger_{std::make_shared<sequence::perceptron>(tagger_prefix)},
      parser_{std::make_shared<parser::sr_parser>(parser_prefix)}
{
    // nothing
}

template <class T>
tree_analyzer<T>::tree_analyzer(const tree_analyzer& other)
    : featurizers_{other.featurizers_},
      stream_{other.stream_->clone()},
      tagger_{other.tagger_},
      parser_{other.parser_}
{
    // nothing
}

template <class T>
void tree_analyzer<T>::add(std::unique_ptr<const tree_featurizer<T>> featurizer)
{
    featurizers_->emplace_back(std::move(featurizer));
}

template <class T>
void tree_analyzer<T>::tokenize(const corpus::document& doc,
                                feature_map& counts)
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

template <class T>
std::unique_ptr<analyzer<T>>
    analyzer_traits<tree_analyzer<T>>::create(const cpptoml::table& global,
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
    auto ana = make_unique<tree_analyzer<T>>(std::move(filts), *tagger_prefix,
                                             *parser_prefix);

    for (const auto& feat : feat_arr->array_of<std::string>())
        ana->add(featurizer_factory<T>::get().create(feat->get()));

    return std::move(ana);
}

template class tree_analyzer<uint64_t>;
template class tree_analyzer<double>;
template struct analyzer_traits<tree_analyzer<uint64_t>>;
template struct analyzer_traits<tree_analyzer<double>>;
}

namespace parser
{
void register_analyzers()
{
    using namespace analyzers;
    register_analyzer<tree_analyzer<uint64_t>>();
    register_analyzer<tree_analyzer<double>>();
}
}
}
