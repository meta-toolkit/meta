/**
 * @file metadata_parser.cpp
 * @author Chase Geigle
 */

#include <cstdlib>

#include "meta/corpus/metadata_parser.h"
#include "meta/io/filesystem.h"
#include "meta/util/shim.h"
#include "meta/util/string_view.h"

namespace meta
{
namespace corpus
{

metadata_parser::metadata_parser(const std::string& filename,
                                 metadata::schema_type schema)
    : infile_{filename}, schema_{std::move(schema)}
{
    // nothing
}

std::vector<metadata::field> metadata_parser::next()
{
    std::vector<metadata::field> mdata;
    std::string str;
    if (infile_)
    {
        std::getline(infile_.stream(), str);
        util::string_view line{str};
        mdata.reserve(schema_.size());
        for (const auto& finfo : schema_)
        {
            if (!infile_ || line.empty())
                throw metadata_exception{
                    "metadata input file ended prematurely"};

            auto delim_pos = line.find('\t');
            auto token = line.substr(0, delim_pos);

            char* end = nullptr;
            switch (finfo.type)
            {
                case metadata::field_type::SIGNED_INT:
                {
                    auto val = std::strtol(token.data(), &end, 10);
                    mdata.emplace_back(static_cast<int64_t>(val));
                    break;
                }

                case metadata::field_type::UNSIGNED_INT:
                {
                    auto val = std::strtoul(token.data(), &end, 10);
                    mdata.emplace_back(static_cast<uint64_t>(val));
                    break;
                }

                case metadata::field_type::DOUBLE:
                {
                    auto val = std::strtod(token.data(), &end);
                    mdata.emplace_back(val);
                    break;
                }

                case metadata::field_type::STRING:
                {
                    mdata.emplace_back(token.to_string());
                    break;
                }
            }

            line = line.substr(std::min(delim_pos, line.size() - 1) + 1);
        }
    }
    return mdata;
}

const metadata::schema_type& metadata_parser::schema() const
{
    return schema_;
}
}
}
