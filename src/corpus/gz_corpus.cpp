/**
 * @file gz_corpus.cpp
 * @author Chase Geigle
 */

#include "corpus/gz_corpus.h"
#include "util/filesystem.h"

namespace meta
{
namespace corpus
{

gz_corpus::gz_corpus(const std::string& file, std::string encoding)
    : corpus{std::move(encoding)},
      cur_id_{0},
      corpus_stream_{file + ".gz"},
      class_stream_{file + ".labels.gz"}
{
    if (!filesystem::file_exists(file + ".numdocs"))
        throw corpus::corpus_exception{
            file + ".numdocs file does not exist (required for gz_corpus)"};

    try
    {
        num_lines_ = std::stoul(filesystem::file_text(file + ".numdocs"));
    }
    catch (const std::exception& ex)
    {
        throw corpus::corpus_exception{"Malformed numdocs file " + file
                                       + ".numdocs: " + ex.what()};
    }
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
    doc.mdata(next_metadata());

    return doc;
}

uint64_t gz_corpus::size() const
{
    return num_lines_;
}
}
}
