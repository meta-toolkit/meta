/**
 * @file metadata_test.cpp
 * @author Sean Massung
 */

#include <fstream>

#include "bandit/bandit.h"
#include "meta/corpus/metadata.h"
#include "meta/corpus/metadata_parser.h"
#include "cpptoml.h"
#include "meta/io/filesystem.h"

using namespace bandit;
using namespace meta;

namespace {

template <class OptionsList>
std::shared_ptr<cpptoml::table>
create_metadata_config(const OptionsList& options) {
    auto base_config = cpptoml::make_table();
    auto metadata_options = cpptoml::make_table_array();
    for (const auto& pair : options) {
        auto option = cpptoml::make_table();
        option->insert("name", pair.first);
        option->insert("type", pair.second);
        metadata_options->push_back(option);
    }
    base_config->insert("metadata", metadata_options);
    return base_config;
}

void create_metadata_file(const std::string& metadata,
                          const std::string& filename) {
    std::ofstream out{filename};
    out << metadata;
}
}

go_bandit([]() {

    describe("[metadata]", []() {
        const std::string filename = "meta-test-metadata.mdata";
        using options_type
            = const std::vector<std::pair<std::string, std::string>>;

        it("should create a parser from a cpptoml table", [&]() {
            options_type options = {{"path", "string"}};
            auto config = create_metadata_config(options);
            const std::string metadata = "/my/path1\n/my/path2"; // no newline
            create_metadata_file(metadata, filename);
            corpus::metadata_parser parser{filename,
                                           corpus::metadata_schema(*config)};
            auto field = parser.next();
            AssertThat(field.size(), Equals(1ul));
            AssertThat(field[0].str, Equals("/my/path1"));
            field = parser.next();
            AssertThat(field.size(), Equals(1ul));
            AssertThat(field[0].str, Equals("/my/path2"));
            filesystem::delete_file(filename);
        });

        it("should read metadata of multiple types", [&]() {
            options_type options = {{"path", "string"},
                                    {"id", "uint"},
                                    {"response", "double"},
                                    {"position", "int"}};
            auto config = create_metadata_config(options);
            const std::string metadata = "/my/path1\t345\t9.345\t7\n"
                                         "/my/path2\t346\t1\t-1\n"
                                         "/my/path3\t347\t-0.4\t0\n";
            create_metadata_file(metadata, filename);
            corpus::metadata_parser parser{filename,
                                           corpus::metadata_schema(*config)};
            const double delta = 0.0000001;
            auto field = parser.next();
            AssertThat(field.size(), Equals(4ul));
            AssertThat(field[0].str, Equals("/my/path1"));
            AssertThat(field[1].usign_int, Equals(345ul));
            AssertThat(field[2].doub, EqualsWithDelta(9.345, delta));
            AssertThat(field[3].sign_int, Equals(7));

            field = parser.next();
            AssertThat(field.size(), Equals(4ul));
            AssertThat(field[0].str, Equals("/my/path2"));
            AssertThat(field[1].usign_int, Equals(346ul));
            AssertThat(field[2].doub, EqualsWithDelta(1.0, delta));
            AssertThat(field[3].sign_int, Equals(-1));

            field = parser.next();
            AssertThat(field.size(), Equals(4ul));
            AssertThat(field[0].str, Equals("/my/path3"));
            AssertThat(field[1].usign_int, Equals(347ul));
            AssertThat(field[2].doub, EqualsWithDelta(-0.4, delta));
            AssertThat(field[3].sign_int, Equals(0));

            filesystem::delete_file(filename);
        });

        it("should read string metadata with spaces", [&]() {
            /// @see https://github.com/meta-toolkit/meta/issues/127
            options_type options = {
                {"path", "string"}, {"title", "string"}, {"comment", "string"}};
            auto config = create_metadata_config(options);

            const std::string metadata
                = "/my/path1\tWonderful Ducklings\ta great children's book\n"
                  "/my/path2\tSo Many Goose\tI saw their tiny little feet";
            create_metadata_file(metadata, filename);

            corpus::metadata_parser parser{filename,
                                           corpus::metadata_schema(*config)};

            auto fields = parser.next();
            AssertThat(fields.size(), Equals(3ul));
            AssertThat(fields[0].type,
                       Equals(corpus::metadata::field_type::STRING));
            AssertThat(fields[0].str, Equals("/my/path1"));
            AssertThat(fields[1].type,
                       Equals(corpus::metadata::field_type::STRING));
            AssertThat(fields[1].str, Equals("Wonderful Ducklings"));
            AssertThat(fields[2].type,
                       Equals(corpus::metadata::field_type::STRING));
            AssertThat(fields[2].str, Equals("a great children's book"));

            fields = parser.next();
            AssertThat(fields.size(), Equals(3ul));
            AssertThat(fields[0].type,
                       Equals(corpus::metadata::field_type::STRING));
            AssertThat(fields[0].str, Equals("/my/path2"));
            AssertThat(fields[1].type,
                       Equals(corpus::metadata::field_type::STRING));
            AssertThat(fields[1].str, Equals("So Many Goose"));
            AssertThat(fields[2].type,
                       Equals(corpus::metadata::field_type::STRING));
            AssertThat(fields[2].str, Equals("I saw their tiny little feet"));

            filesystem::delete_file(filename);
        });
    });
});
