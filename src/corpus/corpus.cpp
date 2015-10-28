/**
 * @file corpus.cpp
 * @author Sean Massung
 */

#include "corpus/corpus.h"
#include "corpus/all.h"
#include "cpptoml.h"
#include "io/filesystem.h"
#include "util/shim.h"

namespace meta
{
namespace corpus
{

corpus::corpus(std::string encoding) : encoding_{std::move(encoding)}
{
    // nothing
}

std::vector<metadata::field> corpus::next_metadata()
{
    return mdata_parser_->next();
}

metadata::schema corpus::schema() const
{
    return mdata_parser_->schema();
}

const std::string& corpus::encoding() const
{
    return encoding_;
}

void corpus::set_metadata_parser(metadata_parser&& parser)
{
    mdata_parser_ = std::move(parser);
}
}
}
