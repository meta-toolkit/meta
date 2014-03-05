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

const std::string character_tokenizer::id = "character-tokenizer";

character_tokenizer::character_tokenizer() : idx_{0}
{
    // nothing
}

void character_tokenizer::set_content(const std::string& content)
{
    idx_ = 0;
    content_ = content;
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
