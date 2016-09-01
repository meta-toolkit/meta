/**
 * @file metadata.h
 * @author Chase Geigle
 *
 * All files in META are dual-licensed under the MIT and NCSA licenses. For more
 * details, consult the file LICENSE.mit and LICENSE.ncsa in the root of the
 * project.
 */

#ifndef META_CORPUS_METADATA_H_
#define META_CORPUS_METADATA_H_

#include <cstdint>
#include <string>
#include <vector>

#include "cpptoml.h"
#include "meta/config.h"
#include "meta/io/packed.h"
#include "meta/util/optional.h"

namespace meta
{
namespace corpus
{

/**
 * Represents the collection of metadata for a document.
 */
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
    using schema_type = std::vector<field_info>;

    metadata(const char* start, const schema_type& sch)
        : schema_{&sch}, start_{start}
    {
        // nothing
    }

    /**
     * @param name The metadata field to obtain
     * @return the metadata associated with that field, if it exists,
     * converted to type T.
     */
    template <class T>
    util::optional<T> get(const std::string& name) const
    {
        metadata_input_stream stream{start_};
        for (uint64_t i = 0; i < schema_->size(); ++i)
        {
            switch ((*schema_)[i].type)
            {
                case field_type::SIGNED_INT:
                {
                    int64_t si;
                    io::packed::read(stream, si);
                    if ((*schema_)[i].name == name)
                        return {field{si}};
                    break;
                }

                case field_type::UNSIGNED_INT:
                {
                    uint64_t ui;
                    io::packed::read(stream, ui);
                    if ((*schema_)[i].name == name)
                        return {field{ui}};
                    break;
                }

                case field_type::DOUBLE:
                {
                    double d;
                    io::packed::read(stream, d);
                    if ((*schema_)[i].name == name)
                        return {field{d}};
                    break;
                }

                case field_type::STRING:
                {
                    std::string s{stream.input_};
                    stream.input_ += s.size() + 1;
                    if ((*schema_)[i].name == name)
                        return {field{std::move(s)}};
                    break;
                }
            }
        }

        return util::nullopt;
    }

    /**
     * Returns the schema for this metadata object.
     */
    const schema_type& schema() const
    {
        return *schema_;
    }

    /**
     * Tagged union to represent a single metadata field.
     */
    struct field
    {
        union {
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

        field(std::string s) : type{field_type::STRING}
        {
            new (&str) std::string(std::move(s));
        }

        field(field&& other) : type{other.type}
        {
            switch (type)
            {
                case field_type::SIGNED_INT:
                    sign_int = other.sign_int;
                    break;

                case field_type::UNSIGNED_INT:
                    usign_int = other.usign_int;
                    break;

                case field_type::DOUBLE:
                    doub = other.doub;
                    break;

                case field_type::STRING:
                    new (&str) std::string(std::move(other.str));
                    break;
            }
        }

        field(const field& other) : type{other.type}
        {
            switch (type)
            {
                case field_type::SIGNED_INT:
                    sign_int = other.sign_int;
                    break;

                case field_type::UNSIGNED_INT:
                    usign_int = other.usign_int;
                    break;

                case field_type::DOUBLE:
                    doub = other.doub;
                    break;

                case field_type::STRING:
                    new (&str) std::string(other.str);
                    break;
            }
        }

        field& operator=(field&& other)
        {
            if (type == field_type::STRING)
                (&str)->~basic_string();

            switch (other.type)
            {
                case field_type::SIGNED_INT:
                    sign_int = other.sign_int;
                    break;

                case field_type::UNSIGNED_INT:
                    usign_int = other.usign_int;
                    break;

                case field_type::DOUBLE:
                    doub = other.doub;
                    break;

                case field_type::STRING:
                    new (&str) std::string(std::move(other.str));
                    break;
            }

            type = other.type;
            return *this;
        }

        field& operator=(const field& other)
        {
            if (type == field_type::STRING)
                (&str)->~basic_string();

            switch (other.type)
            {
                case field_type::SIGNED_INT:
                    sign_int = other.sign_int;
                    break;

                case field_type::UNSIGNED_INT:
                    usign_int = other.usign_int;
                    break;

                case field_type::DOUBLE:
                    doub = other.doub;
                    break;

                case field_type::STRING:
                    new (&str) std::string(other.str);
                    break;
            }

            return *this;
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

  private:
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

    /// pointer to the metadata_file's schema
    const schema_type* schema_;

    /// the start of the metadata within the metadata_file
    const char* start_;
};

/**
 * Extracts a metadata schema from a configuration file.
 * @param config The configuration group that specifies the metadata
 * @return the corresponding metadata::schema object.
 */
metadata::schema_type metadata_schema(const cpptoml::table& config);

/**
 * Exception class for metadata operations.
 */
class metadata_exception : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};
}
}
#endif
