/**
 * @file metadata_parser.cpp
 * @author Chase Geigle
 */

#include "corpus/metadata_parser.h"
#include "io/filesystem.h"

namespace meta
{
namespace corpus
{

metadata_parser::metadata_parser(const std::string& filename,
                                 metadata::schema schema)
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
        mdata.reserve(schema_.size());
        for (const auto& finfo : schema_)
        {
            if (!infile_)
                throw metadata_exception{
                    "metadata input file ended prematurely"};

            infile_ >> str;
            switch (finfo.type)
            {
                case metadata::field_type::SIGNED_INT:
                    mdata.emplace_back(static_cast<int64_t>(std::stol(str)));
                    break;

                case metadata::field_type::UNSIGNED_INT:
                    mdata.emplace_back(static_cast<uint64_t>(std::stoul(str)));
                    break;

                case metadata::field_type::DOUBLE:
                    mdata.emplace_back(std::stod(str));
                    break;

                case metadata::field_type::STRING:
                    mdata.emplace_back(std::move(str));
                    break;
            }
        }
    }
    return mdata;
}

const metadata::schema& metadata_parser::schema() const
{
    return schema_;
}
}
}
