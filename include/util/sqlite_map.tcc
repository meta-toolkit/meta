/**
 * @file sqlite_map.tcc
 * @author Sean Massung
 */

#include "common.h"

namespace meta {
namespace util {

template <class Key, class Value, template <class, class> class Cache>
sqlite_map<Key, Value, Cache>::sqlite_map(const std::string & filename):
    cache_{_max_cached},
    backward_cache_{_max_cached}
{
    if(sqlite3_open_v2(
            filename.c_str(),
            &_db,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE,
            nullptr)
    )
        throw sqlite_map_exception{"failed to open database: " + filename};

    std::string command = "create table if not exists map(";
    command += "id "    + sql_type<Key>()   + " primary key not null, ";
    command += "value " + sql_type<Value>() + " not null);";
    command += "CREATE INDEX IF NOT EXISTS map_values ON map(value DESC);";

    if(sqlite3_exec(_db, command.c_str(),
                    nullptr, nullptr, nullptr) != SQLITE_OK)
        throw sqlite_map_exception{"error running command: " + command};

    if (sqlite3_exec(_db, "PRAGMA synchronous = off;",
                     nullptr, nullptr, nullptr) != SQLITE_OK)
        throw sqlite_map_exception{"failed to set db properties"};

    if (sqlite3_exec(_db, "PRAGMA journal_mode = MEMORY;",
                     nullptr, nullptr, nullptr) != SQLITE_OK)
        throw sqlite_map_exception{"failed to set db properties"};

    sqlite3_prepare_v2(_db,
                       "INSERT OR IGNORE INTO map (id, value) VALUES (?1, ?2);"
                       "UPDATE map SET value = ?2 WHERE id = ?1;",
                       -1,
                       &insert_stmt_,
                       nullptr);

    sqlite3_prepare_v2(_db,
                       "SELECT value FROM map WHERE id = ?;",
                       -1,
                       &find_stmt_,
                       nullptr);

    sqlite3_prepare_v2(_db,
                       "SELECT id FROM map WHERE value = ?;",
                       -1,
                       &find_key_stmt_,
                       nullptr);

    sqlite3_prepare_v2(_db,
                       "SELECT COUNT(*) FROM map;",
                       -1,
                       &size_stmt_,
                       nullptr);

    sqlite_binder{size_stmt_};
    if (sqlite3_step(size_stmt_) != SQLITE_ROW)
        throw sqlite_map_exception{"failed to get size"};
    _size = sqlite3_column_int64(size_stmt_, 0);
    if (sqlite3_step(size_stmt_) != SQLITE_DONE)
        throw sqlite_map_exception{"size returned too much data"};

    cache_.on_drop([&](const Key & key, const Value & value) {
        std::lock_guard<std::mutex> lock{_mutex};
        sqlite_binder bind{insert_stmt_, key, value};
        if (sqlite3_step(insert_stmt_) != SQLITE_DONE)
            throw sqlite_map_exception{"insert failed: "
                                       + common::to_string(key)
                                       + ", "
                                       + common::to_string(value)};
    });
}

template <class Key, class Value, template <class, class> class Cache>
template <class T>
std::string sqlite_map<Key, Value, Cache>::sql_type() const
{
    if (std::is_same<std::string, T>::value)
        return "text";
    if (std::is_floating_point<T>::value)
        return "double precision";
    return "bigint";
}

template <class Key, class Value, template <class, class> class Cache>
sqlite_map<Key, Value, Cache>::~sqlite_map()
{
    cache_.clear(); // ensure that the cache gets flushed before we
                    // close the connection
    sqlite3_finalize(insert_stmt_);
    sqlite3_finalize(find_stmt_);
    sqlite3_close(_db);
}

template <class Key, class Value, template <class, class> class Cache>
void sqlite_map<Key, Value, Cache>::insert(const Key & key, const Value & value)
{
    std::lock_guard<std::mutex> lock{_mutex};
    // check if map size will increase
    auto cache_result = cache_.find(key);
    if (!cache_result) {
        sqlite_binder bind{find_stmt_, key};
        if (sqlite3_step(find_stmt_) != SQLITE_ROW)
            _size++; // not in cache or in db
    }
    // insert into or overwrite in the write-back cache for committing later
    cache_.insert(key, value);
}

template <class Key, class Value, template <class, class> class Cache>
util::optional<Value> sqlite_map<Key, Value, Cache>::find(const Key & key) const
{
    auto cache_result = cache_.find(key);
    if (cache_result)
        return cache_result;

    Value result;
    {
        std::lock_guard<std::mutex> lock{_mutex};
        sqlite_binder bind{find_stmt_, key};
        if (sqlite3_step(find_stmt_) != SQLITE_ROW)
            return util::nullopt;
        result = sql_fetch_result<Value>(find_stmt_, 0);
        if (sqlite3_step(find_stmt_) != SQLITE_DONE)
            throw sqlite_map_exception{"find for value produced too much data: "
                                       + common::to_string(key)};
    }
    cache_.insert(key, result);
    return result;
}

template <class Key, class Value, template <class, class> class Cache>
util::optional<Key>
sqlite_map<Key, Value, Cache>::find_key(const Value & value) const
{
    cache_.clear(); // need to flush writes in order for us to find by value
    auto cache_result = backward_cache_.find(value);
    if (cache_result)
        return cache_result;

    Key result;
    {
        std::lock_guard<std::mutex> lock{_mutex};
        sqlite_binder{find_key_stmt_, value};
        if (sqlite3_step(find_key_stmt_) != SQLITE_ROW)
            return util::nullopt;
        result = sql_fetch_result<Key>(find_key_stmt_, 0);
        if (sqlite3_step(find_key_stmt_) != SQLITE_DONE)
            throw sqlite_map_exception{"find for key produced too much data: "
                                       + common::to_string(value)};
    }
    backward_cache_.insert(value, result);
    return result;
}

template <>
inline uint64_t sql_fetch_result<uint64_t>(sqlite3_stmt * stmt, int idx)
{
    return sqlite3_column_int64(stmt, idx);
}

template <>
inline double sql_fetch_result<double>(sqlite3_stmt * stmt, int idx)
{
    return sqlite3_column_double(stmt, idx);
}

template <>
inline std::string sql_fetch_result<std::string>(sqlite3_stmt * stmt, int idx)
{
    return reinterpret_cast<const char *>(sqlite3_column_text(stmt, idx));
}

template <class Key, class Value, template <class, class> class Cache>
uint64_t sqlite_map<Key, Value, Cache>::size() const
{
    return _size;
}

template <class Key, class Value, template <class, class> class Cache>
template <class Result, class... Args>
auto sqlite_map<Key, Value, Cache>::query(const std::string & query,
                                          Args &&... args)
-> query_result<Result>
{
    cache_.clear(); // ensure results are flushed before making the query
    sqlite3_stmt * stmt;
    sqlite3_prepare_v2(_db, query.c_str(), -1, &stmt, nullptr);
    sqlite_binder::bind_values(stmt, 1, args...);
    return {stmt};
}

}
}
