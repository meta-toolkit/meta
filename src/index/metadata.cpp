/**
 * @file metadata.cpp
 * @author Chase Geigle
 */

#include "index/metadata.h"

namespace meta
{
namespace index
{

metadata::schema metadata_schema(const cpptoml::table& config)
{
    metadata::schema schema;
    if (auto metadata = config.get_table_array("metadata"))
    {
        const auto& arr = metadata->get();
        schema.reserve(arr.size());
        for (const auto& table : arr)
        {
            auto name = table->get_as<std::string>("name");
            auto type = table->get_as<std::string>("type");

            if (!name)
                throw metadata::exception{"name needed for metadata field"};

            if (!type)
                throw metadata::exception{"type needed for metadata field"};

            index::metadata::field_type ftype;
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
                throw metadata::exception{"invalid metadata type: \"" + *type
                                          + "\""};
            }
            schema.emplace_back(*name, ftype);
        }
    }
    return schema;
}
}
}
