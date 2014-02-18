/**
 * @file character_tokenizer.cpp
 * @author Chase Geigle
 */

#include "analyzers/tokenizers/character_tokenizer.h"
#include "corpus/document.h"
#include "io/mmap_file.h"

namespace meta
{
namespace analyzers
{

character_tokenizer::character_tokenizer(corpus::document& doc)
    : idx_{0}
{
    if (doc.contains_content())
        content_ = doc.content();
    else
    {
        io::mmap_file file{doc.path()};
        content_ = {file.begin(), file.begin() + file.size()};
    }
}

std::string character_tokenizer::next()
{
    if (!*this)
        throw token_stream_exception{"next() called with no tokens left"};

    return {1, content_[idx_++]};
}

character_tokenizer::operator bool() const
{
    return idx_ < content_.size();
}

}
}
