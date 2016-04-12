/**
 * @file metadata.cpp
 * @author Chase Geigle
 */

#include "meta/corpus/metadata.h"

namespace meta
{
namespace corpus
{

metadata::schema_type metadata_schema(const cpptoml::table& config)
{
    metadata::schema_type schema;
    if (auto metadata = config.get_table_array("metadata"))
    {
        const auto& arr = metadata->get();
        schema.reserve(arr.size());
        for (const auto& table : arr)
        {
            auto name = table->get_as<std::string>("name");
            auto type = table->get_as<std::string>("type");

            if (!name)
                throw metadata_exception{"name needed for metadata field"};

            if (!type)
                throw metadata_exception{"type needed for metadata field"};

            metadata::field_type ftype;
            if (*type == "int")
            {
                ftype = metadata::field_type::SIGNED_INT;
            }
            else if (*type == "uint")
            {
                ftype = metadata::field_type::UNSIGNED_INT;
            }
            else if (*type == "double")
            {
                ftype = metadata::field_type::DOUBLE;
            }
            else if (*type == "string")
            {
                ftype = metadata::field_type::STRING;
            }
            else
            {
                throw metadata_exception{"invalid metadata type: \"" + *type
                                         + "\""};
            }
            schema.emplace_back(*name, ftype);
        }
    }
    return schema;
}
}
}
