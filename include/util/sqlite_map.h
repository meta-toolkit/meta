/**
 * @file sqlite_map.h
 * @author Sean Massung
 * @author Chase Geigle
 *
 * All files in META are released under the MIT license. For more details,
 * consult the file LICENSE in the root of the project.
 */

#ifndef _SQLITE_MAP_H_
#define _SQLITE_MAP_H_

#include <mutex>
#include <type_traits>
#include <string>
#include "meta.h"
#include "sqlite3.h"
#include "util/optional.h"
#include "caching/splay_cache.h"

namespace meta {
namespace util {

/**
 * Wrapper for a sqlite3 database as a simple dictionary. Keys and values can be
 * integral types, floating point, or strings. Note that the strings currently
 * don't support containing spaces.
 */
template <class Key, class Value, class Cache = caching::splay_cache<Key, Value>>
class sqlite_map
{
    static_assert(
        std::is_integral<Key>::value ||
        std::is_floating_point<Key>::value ||
        std::is_base_of<util::numeric, Key>::value ||
        std::is_same<std::string, Key>::value,
        "sqlite_map templated types must be integral types or string types"
    );

    static_assert(
        std::is_integral<Value>::value ||
        std::is_floating_point<Value>::value ||
        std::is_base_of<util::numeric, Value>::value ||
        std::is_same<std::string, Value>::value,
        "sqlite_map templated types must be integral types or string types"
    );

    public:
        /**
         * @param filename The file where the map is stored on disk
         */
        sqlite_map(const std::string & filename);

        /**
         * Destructor.
         */
        ~sqlite_map();

        /**
         * If the key already exists, the map is unchanged.
         * @param key The key to insert
         * @param value The value to insert
         */
        void insert(const Key & key, const Value & value);

        /**
         * @param key The key of the item to search for
         * @return the value corresponding to this key
         */
        util::optional<Value> find(const Key & key) const;

        /**
         * @param value The value for the item to search for
         * @return The key associated with this value
         */
        util::optional<Key> find(const Value & value) const;

        /**
         * @return the number of elements (rows) in this map
         */
        uint64_t size() const;

    private:
        /** pointer to the database; sqlite3 handles memory for this object */
        sqlite3* _db;

        /** prepared statement for inserts */
        sqlite3_stmt * insert_stmt_;

        /** prepared statement for finds */
        sqlite3_stmt * find_stmt_;

        /** prepared statement for finds of keys by value */
        sqlite3_stmt * find_key_stmt_;

        /** prepared statement for size */
        sqlite3_stmt * size_stmt_;

        /** cache in front of the db for performance */
        mutable Cache cache_;

        /** the number of elements in this map (calculated by sqlite) */
        uint64_t _size;

        /** a mutex for synchronization */
        mutable std::mutex _mutex;

        /** after this number is reached, commit inserts and updates */
        const static uint64_t _max_cached = 100000;

        /**
         * A RAII-style class for binding values to a sql query. Ensures
         * that sqlite3_reset() is properly invoked on any prepared
         * statement.
         */
        class sqlite_binder
        {
            sqlite3_stmt * stmt_;

            public:
                template <class... Args>
                sqlite_binder(sqlite3_stmt * stmt, Args &&... args)
                    : stmt_{stmt}
                {
                    bind_values(1, args...);
                }

                template <class T, class... Args>
                void bind_values(int idx, T && value, Args &&... args) {
                    bind_value(idx, value);
                    bind_values(idx + 1, args...);
                }

                void bind_values(int) {}

                void bind_value(int idx, const std::string & value) {
                    sqlite3_bind_text(stmt_, idx, value.c_str(), -1, nullptr);
                }

                void bind_value(int idx, uint64_t value) {
                    sqlite3_bind_int64(stmt_, idx, value);
                }

                void bind_value(int idx, double value) {
                    sqlite3_bind_double(stmt_, idx, value);
                }

                ~sqlite_binder() {
                    sqlite3_reset(stmt_);
                }
        };

        /**
         * @return the type that the C++ object would be stored in a SQL
         * database as
         */
        template <class T>
        std::string sql_type() const;

    public:
        /**
         * Basic exception for sqlite_map.
         */
        class sqlite_map_exception: public std::exception
        {
            public:
                sqlite_map_exception(const std::string & error):
                    _error(error) { /* nothing */ }

                const char* what () const throw ()
                {
                    return _error.c_str();
                }

            private:
                std::string _error;
        };
};

/**
 * Helper function for fetching results from prepared statements.
 */
template <class Result>
Result sql_fetch_result(sqlite3_stmt * stmt, int idx);

}
}

#include "sqlite_map.tcc"
#endif
