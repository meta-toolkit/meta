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
#include "common.h"
#include "meta.h"
#include "sqlite3.h"
#include "util/optional.h"
#include "caching/splay_cache.h"

namespace meta {
namespace util {

/**
 * Helper function for fetching results from prepared statements.
 */
template <class Result>
Result sql_fetch_result(sqlite3_stmt * stmt, int idx);

/**
 * Wrapper for a sqlite3 database as a simple dictionary. Keys and values can be
 * integral types, floating point, or strings. Note that the strings currently
 * don't support containing spaces.
 */
template <class Key, class Value, template <class, class> class Cache = caching::splay_cache>
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
        util::optional<Key> find_key(const Value & value) const;

        /**
         * @return the number of elements (rows) in this map
         */
        uint64_t size() const;

        template <class Result>
        class query_result {
            sqlite3_stmt * stmt_;

            int step() {
                return sqlite3_step(stmt_);
            }

            Result fetch() {
                return sql_fetch_result<Result>(stmt_, 0);
            }

            public:
                query_result(sqlite3_stmt * stmt) : stmt_{stmt}
                { /* nothing */ }

                ~query_result() {
                    sqlite3_finalize(stmt_);
                }

                class iterator {
                    friend query_result;

                    int return_code_;
                    query_result * qr_;
                    Result result_;

                    iterator(int ret, query_result * qr)
                        : return_code_{ret}, qr_{qr}, result_{qr_->fetch()}
                    { /* nothing */ }

                    public:
                        friend bool operator==(const iterator & first,
                                               const iterator & second) {
                            if (first.return_code_ == SQLITE_DONE)
                                return second.return_code_ == SQLITE_DONE;
                            return first.return_code_ == second.return_code_
                                   && first.result_ == second.result_;
                        }

                        friend bool operator!=(const iterator & first,
                                               const iterator & second) {
                            return !(first == second);
                        }

                        iterator & operator++() {
                            return_code_ = qr_->step();
                            if (return_code_ == SQLITE_ROW)
                                result_ = qr_->fetch();
                            else if (return_code_ != SQLITE_DONE)
                                throw sqlite_map_exception{"sql error in iteration over query set: "
                                    + common::to_string(return_code_)};
                            return *this;
                        }

                        Result operator*() {
                            return result_;
                        }
                };

                iterator begin() {
                    return {step(), this};
                }

                iterator end() {
                    return {SQLITE_DONE, this};
                }
        };

        template <class Result, class... Args>
        query_result<Result> query(const std::string & query, Args &&... args);

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
        mutable Cache<Key, Value> cache_;

        /** cache in front of the db for performance, reverse direction */
        mutable Cache<Value, Key> backward_cache_;

        /** a mutex for synchronization */
        mutable std::mutex _mutex;

        /** after this number is reached, commit inserts and updates */
        const static uint64_t _max_cached = 100000;

        /** the size of the map */
        uint64_t _size{0};

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
                    bind_values(stmt_, 1, args...);
                }

                template <class T, class... Args>
                static void bind_values(sqlite3_stmt * stmt,
                                int idx,
                                T && value,
                                Args &&... args) {
                    bind_value(stmt, idx, value);
                    bind_values(stmt, idx + 1, args...);
                }

                static void bind_values(sqlite3_stmt *, int) {}

                static void bind_value(sqlite3_stmt * stmt,
                                       int idx,
                                       const std::string & value) {
                    sqlite3_bind_text(stmt, idx, value.c_str(), -1, nullptr);
                }

                static void bind_value(sqlite3_stmt * stmt,
                                       int idx,
                                       uint64_t value) {
                    sqlite3_bind_int64(stmt, idx, value);
                }

                static void bind_value(sqlite3_stmt * stmt,
                                       int idx,
                                       double value) {
                    sqlite3_bind_double(stmt, idx, value);
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


}
}

#include "sqlite_map.tcc"
#endif
