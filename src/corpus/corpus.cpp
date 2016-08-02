/**
 * @file corpus.cpp
 * @author Sean Massung
 */

#include "cpptoml.h"
#include "meta/corpus/all.h"
#include "meta/corpus/corpus.h"
#include "meta/io/filesystem.h"
#include "meta/util/shim.h"

namespace meta
{
namespace corpus
{

corpus::corpus(std::string encoding)
    : encoding_{std::move(encoding)}, store_full_text_{false}
{
    // nothing
}

std::vector<metadata::field> corpus::next_metadata()
{
    return mdata_parser_->next();
}

metadata::schema_type corpus::schema() const
{
    auto schema = mdata_parser_->schema();
    if (store_full_text())
        schema.insert(
            schema.begin(),
            metadata::field_info{"content", metadata::field_type::STRING});
    return schema;
}

const std::string& corpus::encoding() const
{
    return encoding_;
}

void corpus::set_metadata_parser(metadata_parser&& parser)
{
    mdata_parser_ = std::move(parser);
}

void corpus::set_store_full_text(bool store_full_text)
{
    store_full_text_ = store_full_text;
}

bool corpus::store_full_text() const
{
    return store_full_text_;
}
}
}
