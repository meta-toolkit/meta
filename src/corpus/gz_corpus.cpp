/**
 * @file gz_corpus.cpp
 * @author Chase Geigle
 */

#include "meta/corpus/gz_corpus.h"
#include "meta/io/filesystem.h"
#include "meta/util/shim.h"

namespace meta
{
namespace corpus
{

const util::string_view gz_corpus::id = "gz-corpus";

gz_corpus::gz_corpus(const std::string& file, std::string encoding,
                     uint64_t num_docs)
    : corpus{std::move(encoding)},
      cur_id_{0},
      num_lines_{num_docs},
      corpus_stream_{file + ".gz"},
      class_stream_{file + ".labels.gz"}
{
    // nothing
}

bool gz_corpus::has_next() const
{
    return cur_id_ != num_lines_;
}

document gz_corpus::next()
{
    class_label label{"[none]"};

    if (class_stream_)
        std::getline(class_stream_, static_cast<std::string&>(label));

    std::string line;
    std::getline(corpus_stream_, line);

    document doc{cur_id_++, label};
    doc.content(line, encoding());

    auto mdata = next_metadata();
    if (store_full_text())
        mdata.insert(mdata.begin(), metadata::field{doc.content()});
    doc.mdata(std::move(mdata));

    return doc;
}

uint64_t gz_corpus::size() const
{
    return num_lines_;
}

template <>
std::unique_ptr<corpus> make_corpus<gz_corpus>(util::string_view prefix,
                                               util::string_view dataset,
                                               const cpptoml::table& config)
{
    auto encoding = config.get_as<std::string>("encoding").value_or("utf-8");

    auto num_docs = config.get_as<int64_t>("num-docs");
    if (!num_docs)
        throw corpus_exception{"num-docs config param required for gz_corpus"};

    // string_view doesn't have operator+ overloads...
    auto filename = prefix.to_string();
    filename += "/";
    filename.append(dataset.data(), dataset.size());
    filename += "/";
    filename.append(dataset.data(), dataset.size());
    filename += ".dat";

    return make_unique<gz_corpus>(filename, encoding,
                                  static_cast<uint64_t>(*num_docs));
}
}
}
