/**
 * @file metadata.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_INDEX_METADATA_H_
#define META_INDEX_METADATA_H_

#include <cstdint>
#include <vector>
#include <string>

#include "cpptoml.h"
#include "io/binary.h"

namespace meta
{
namespace index
{

class metadata
{
  public:
    /**
     * Type tag for a field.
     */
    enum class field_type : uint8_t
    {
        SIGNED_INT = 0,
        UNSIGNED_INT,
        DOUBLE,
        STRING
    };

    /**
     * Pair for storing the schema: contains its name and type.
     */
    struct field_info
    {
        std::string name;
        field_type type;

        field_info() = default;
        field_info(std::string n, field_type ft) : name{std::move(n)}, type{ft}
        {
            // nothing
        }
        field_info(const field_info&) = default;
        field_info(field_info&&) = default;
        field_info& operator=(const field_info&) = default;
        field_info& operator=(field_info&&) = default;
        ~field_info() = default;
    };

    // I want the below to be a const field_info, but g++ gives a cryptic
    // compiler error in that case... clang++ accepts it just fine. -sigh-
    using schema = std::vector<field_info>;

    metadata(const char* start, const schema& sch)
        : schema_{sch}, stream_{start}
    {
        // nothing
    }

    template <class T>
    T get(const std::string& name)
    {
        for (uint64_t i = 0; i < stored_fields_.size(); ++i)
        {
            if (schema_[i].name == name)
                return stored_fields_[i];
        }

        for (uint64_t i = stored_fields_.size(); i < schema_.size(); ++i)
        {
            switch (schema_[i].type)
            {
                case field_type::SIGNED_INT:
                    int64_t si;
                    io::read_packed_binary(stream_, si);
                    stored_fields_.emplace_back(si);
                    break;
                case field_type::UNSIGNED_INT:
                    uint64_t ui;
                    io::read_packed_binary(stream_, ui);
                    stored_fields_.emplace_back(ui);
                    break;
                case field_type::DOUBLE:
                    double d;
                    io::read_packed_binary(stream_, d);
                    stored_fields_.emplace_back(d);
                    break;
                case field_type::STRING:
                    std::string s{stream_.input_};
                    stream_.input_ += s.size() + 1;
                    stored_fields_.emplace_back(std::move(s));
                    break;
            }

            if (schema_[i].name == name)
                return stored_fields_[i];
        }

        throw exception{"metadata column \"" + name + "\" not found"};
    }

    class exception : public std::runtime_error
    {
      public:
        using std::runtime_error::runtime_error;
    };

  private:
    /**
     * Tagged union to represent a single metadata field.
     */
    struct field
    {
        union
        {
            int64_t sign_int;
            uint64_t usign_int;
            double doub;
            std::string str;
        };

        field_type type;

        field(int64_t sgn) : sign_int{sgn}, type{field_type::SIGNED_INT}
        {
            // nothing
        }

        field(uint64_t usgn) : usign_int{usgn}, type{field_type::UNSIGNED_INT}
        {
            // nothing
        }

        field(double d) : doub{d}, type{field_type::DOUBLE}
        {
            // nothing
        }

        field(std::string&& s) : type{field_type::STRING}
        {
            new (&str) std::string(std::move(s));
        }

        ~field()
        {
            // invoke string destructor if needed
            if (type == field_type::STRING)
                (&str)->~basic_string();
        }

        operator int64_t() const
        {
            return sign_int;
        }

        operator uint64_t() const
        {
            return usign_int;
        }

        operator double() const
        {
            return doub;
        }

        operator std::string() const
        {
            return str;
        }
    };

    struct metadata_input_stream
    {
        metadata_input_stream(const char* input) : input_{input}
        {
            // nothing
        }

        char get()
        {
            return *input_++;
        }

        const char* input_;
    };

    /// reference to the metadata_file's schema
    const schema& schema_;

    /// the fake input stream used for read_packed_binary
    metadata_input_stream stream_;

    /// storage for decoded fields
    std::vector<field> stored_fields_;
};

/**
 * Extracts a metadata schema from a configuration file.
 * @param config The configuration group that specifies the metadata
 * @return the corresponding metadata::schema object.
 */
metadata::schema metadata_schema(const cpptoml::table& config);
}
}
#endif
