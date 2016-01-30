/**
 * @file metadata_file.cpp
 * @author Chase Geigle
 */

#include "meta/index/metadata_file.h"
#include "meta/io/packed.h"

namespace meta
{
namespace index
{

namespace
{
struct char_input_stream
{
    char_input_stream(const char* input, const char* end)
        : input_{input}, end_{end}
    {
        // nothing
    }

    char get()
    {
        if (input_ == end_)
            throw corpus::metadata_exception{
                "seeking past end of metadata file"};

        return *input_++;
    }

    const char* input_;
    const char* end_;
};
}

metadata_file::metadata_file(const std::string& prefix)
    : index_{prefix + "/metadata.index"}, md_db_{prefix + "/metadata.db"}
{
    // read in the header to populate the schema
    char_input_stream stream{md_db_.begin(), md_db_.begin() + md_db_.size()};
    uint64_t num_fields;
    io::packed::read(stream, num_fields);

    schema_.reserve(num_fields);
    for (uint64_t i = 0; i < num_fields; ++i)
    {
        corpus::metadata::field_info info;
        info.name = std::string{stream.input_};
        stream.input_ += info.name.size() + 1;
        static_assert(sizeof(corpus::metadata::field_type) == sizeof(uint8_t),
                      "metadata::field_type size not updated in metadata_file");
        info.type = static_cast<corpus::metadata::field_type>(stream.get());
        schema_.emplace_back(std::move(info));
    }
}

corpus::metadata metadata_file::get(doc_id d_id) const
{
    if (d_id >= index_.size())
        throw corpus::metadata_exception{
            "invalid doc id in metadata retrieval"};

    uint64_t seek_pos = index_[d_id];
    return {md_db_.begin() + seek_pos, schema_};
}

uint64_t metadata_file::size() const
{
    return index_.size();
}
}
}
