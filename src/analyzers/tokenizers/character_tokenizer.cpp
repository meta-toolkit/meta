/**
 * @file character_tokenizer.cpp
 * @author Chase Geigle
 */

#include "meta/analyzers/tokenizers/character_tokenizer.h"
#include "meta/corpus/document.h"
#include "meta/io/mmap_file.h"

namespace meta
{
namespace analyzers
{
namespace tokenizers
{

const util::string_view character_tokenizer::id = "character-tokenizer";

character_tokenizer::character_tokenizer() : idx_{0}
{
    // nothing
}

void character_tokenizer::set_content(std::string&& content)
{
    idx_ = 0;
    content_ = std::move(content);
}

std::string character_tokenizer::next()
{
    if (!*this)
        throw token_stream_exception{"next() called with no tokens left"};

    return {content_[idx_++]};
}

character_tokenizer::operator bool() const
{
    return idx_ < content_.size();
}
}
}
}
