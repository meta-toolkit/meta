/**
 * @file metadata_file.cpp
 * @author Chase Geigle
 */

#include "index/metadata_file.h"
#include "io/binary.h"

namespace meta
{
namespace index
{

namespace
{
struct char_input_stream
{
    char_input_stream(const char* input) : input_{input}
    {
        // nothing
    }

    char get()
    {
        return *input_++;
    }

    const char* input_;
};
}

metadata_file::metadata_file(const std::string& prefix)
    : index_{prefix + "/metadata.index"}, md_db_{prefix + "/metadata.db"}
{
    // read in the header to populate the schema
    char_input_stream stream{md_db_.begin()};
    uint64_t num_fields;
    io::read_packed_binary(stream, num_fields);

    schema_.reserve(num_fields);
    for (uint64_t i = 0; i < num_fields; ++i)
    {
        metadata::field_info info;
        info.name = std::string{stream.input_};
        stream.input_ += info.name.size() + 1;
        io::read_packed_binary(stream, info.type);
        schema_.emplace_back(std::move(info));
    }
}

metadata metadata_file::get(doc_id d_id) const
{
    if (d_id >= index_.size())
        throw metadata::exception{"invalid doc id in metadata retrieval"};

    uint64_t seek_pos = index_[d_id];
    return {md_db_.begin() + seek_pos, schema_};
}
}
}
