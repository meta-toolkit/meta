/**
 * @file metadata_writer.cpp
 * @author Chase Geigle
 */

#include "meta/index/metadata_writer.h"
#include "meta/io/packed.h"

namespace meta
{
namespace index
{

metadata_writer::metadata_writer(const std::string& prefix, uint64_t num_docs,
                                 corpus::metadata::schema_type schema)
    : seek_pos_{prefix + "/metadata.index", num_docs},
      byte_pos_{0},
      db_file_{prefix + "/metadata.db", std::ios::binary},
      schema_{std::move(schema)}
{
    // write metadata header
    // cast below is needed for OS X overload resolution
    byte_pos_ += io::packed::write(db_file_,
                                   static_cast<uint64_t>(schema_.size() + 2));

    byte_pos_ += io::packed::write(db_file_, std::string{"length"});
    byte_pos_ += io::packed::write(db_file_,
                                   corpus::metadata::field_type::UNSIGNED_INT);

    byte_pos_ += io::packed::write(db_file_, "unique-terms");
    byte_pos_ += io::packed::write(db_file_,
                                   corpus::metadata::field_type::UNSIGNED_INT);

    for (const auto& finfo : schema_)
    {
        byte_pos_ += io::packed::write(db_file_, finfo.name);
        byte_pos_ += io::packed::write(db_file_, finfo.type);
    }
}

void metadata_writer::write(doc_id d_id, uint64_t length, uint64_t num_unique,
                            const std::vector<corpus::metadata::field>& mdata)
{
    std::lock_guard<std::mutex> lock{lock_};

    seek_pos_[d_id] = byte_pos_;
    // write "mandatory" metadata
    byte_pos_ += io::packed::write(db_file_, length);
    byte_pos_ += io::packed::write(db_file_, num_unique);

    // write optional metadata
    if (mdata.size() != schema_.size())
        throw corpus::metadata_exception{
            "schema mismatch when writing metadata"};

    for (const auto& fld : mdata)
    {
        switch (fld.type)
        {
            case corpus::metadata::field_type::SIGNED_INT:
                byte_pos_ += io::packed::write(db_file_, fld.sign_int);
                break;

            case corpus::metadata::field_type::UNSIGNED_INT:
                byte_pos_ += io::packed::write(db_file_, fld.usign_int);
                break;

            case corpus::metadata::field_type::DOUBLE:
                byte_pos_ += io::packed::write(db_file_, fld.doub);
                break;

            case corpus::metadata::field_type::STRING:
                byte_pos_ += io::packed::write(db_file_, fld.str);
                break;
        }
    }
}
}
}
