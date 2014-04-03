/**
 * @file parser.cpp
 */

#include "io/mmap_file.h"
#include "io/parser.h"
#include "util/shim.h"

namespace meta
{
namespace io
{

parser::parser(const std::string& input, const std::string& delims,
               input_type in_type /* = File */)
    : idx_{0}
{
    // initialize delimiter array
    invalid_.fill(false);
    for (const auto& ch : delims)
        invalid_[static_cast<uint8_t>(ch)] = true;

    // determine whether we're parsing an mmap_file or a std::string
    if (in_type == input_type::File)
    {
        filename_ = input;
        mmap_file_ = make_unique<io::mmap_file>(input);
        data_ = mmap_file_->begin();
        size_ = mmap_file_->size();
    }
    else /* in_type == input_type::String */
    {
        filename_ = "";
        data_ = input.data();
        size_ = input.size();
    }

    get_next();
}

parser::parser(parser&&) = default;
parser::~parser() = default;
parser& parser::operator=(parser&&) = default;

void parser::get_next()
{
    next_ = util::nullopt;
    for (size_t i = idx_; i < size_; ++i)
    {
        if (invalid_[static_cast<uint8_t>(data_[i])])
        {
            next_ = std::string{data_ + idx_, i - idx_};
            idx_ = i + 1;
            return;
        }
    }

    // get last token if there is no delimiter at the EOF
    if (idx_ != size_)
    {
        next_ = std::string{data_ + idx_, size_ - idx_};
        idx_ = size_;
    }
}

std::string parser::filename() const
{
    return filename_;
}

std::string parser::peek() const
{
    return *next_;
}

std::string parser::next()
{
    std::string ret{*next_};
    get_next();
    return ret;
}

bool parser::has_next() const
{
    return static_cast<bool>(next_);
}
}
}
