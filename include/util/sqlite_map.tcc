/**
 * @file sqlite_map.tcc
 * @author Sean Massung
 */

#include "common.h"

namespace meta {
namespace util {

template <class Key, class Value>
sqlite_map<Key, Value>::sqlite_map(const std::string & filename):
    _sql_key_type{sql_type<Key>()},
    _sql_value_type{sql_type<Value>()},
    _size{0},
    _commands{""},
    _size_dirty{true}, // always force recalculation when a db is opened
    _num_cached{0}
{
    if(sqlite3_open_v2(
            filename.c_str(),
            &_db,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE,
            nullptr)
    )
        throw sqlite_map_exception{"failed to open database: " + filename};

    std::string command = "create table if not exists map(";
    command += "id "    + _sql_key_type   + " primary key not null, ";
    command += "value " + _sql_value_type + " not null);";

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
                       "INSERT OR IGNORE INTO map (id, value) VALUES (?, ?);",
                       -1,
                       &insert_stmt_,
                       nullptr);

    sqlite3_prepare_v2(_db,
                       "SELECT value FROM map WHERE id = ?;",
                       -1,
                       &find_stmt_,
                       nullptr);

    sqlite3_prepare_v2(_db,
                       "SELECT COUNT(*) FROM map;",
                       -1,
                       &size_stmt_,
                       nullptr);

    _commands.reserve(100000);
}

template <class Key, class Value>
template <class T>
std::string sqlite_map<Key, Value>::sql_type() const
{
    if(std::is_same<std::string, T>::value)
        return "text";
    if(std::is_floating_point<T>::value)
        return "double precision";
    return "bigint";
}

template <class Key, class Value>
template <class T>
std::string sqlite_map<Key, Value>::sql_text(const T & elem) const
{
    std::string text = common::to_string(elem);
    if(std::is_same<T, std::string>::value)
        text = "\"" + text + "\"";
    return text;
}

template <class Key, class Value>
sqlite_map<Key, Value>::~sqlite_map()
{
    commit();
    sqlite3_finalize(insert_stmt_);
    sqlite3_finalize(find_stmt_);
    sqlite3_close(_db);
}

template <class Key, class Value>
void sqlite_map<Key, Value>::insert(const Key & key, const Value & value)
{
    /*
    std::string command = "insert into map (id, value) select ";
    command += sql_text(key) + "," + sql_text(value);
    command += " where not exists (select value from map where id = ";
    command += sql_text(key) + ");";

    std::lock_guard<std::mutex> lock{_mutex};
    _size_dirty = true;
    _commands += command;
    if(_num_cached++ >= _max_cached)
    {
        commit();
        _num_cached = 0;
    }
    */

    std::lock_guard<std::mutex> lock{_mutex};
    sqlite_binder bind{insert_stmt_, key, value};
    if (sqlite3_step(insert_stmt_) != SQLITE_DONE)
        throw sqlite_map_exception{"insert failed: "
                                   + common::to_string(key)
                                   + ", "
                                   + common::to_string(value)};
}

template <class Key, class Value>
void sqlite_map<Key, Value>::commit()
{
    if(_commands.empty())
        return;

    _commands = "begin; " + _commands + " commit;";

    if(sqlite3_exec(_db, _commands.c_str(),
            nullptr, nullptr, nullptr) != SQLITE_OK)
        throw sqlite_map_exception{"error running commit command"};

    _commands.clear();
}

template <class Key, class Value>
Value sqlite_map<Key, Value>::find(const Key & key)
{
    /*
    std::string command = "select value from map where id = ";
    command += sql_text(key);

    std::lock_guard<std::mutex> lock{_mutex};
    commit();

    if(sqlite3_exec(_db, command.c_str(), find_callback,
            (void*) this, nullptr) != SQLITE_OK)
        throw sqlite_map_exception{"error running command: " + command};

    return _return_value;
    */

    std::lock_guard<std::mutex> lock{_mutex};
    sqlite_binder bind{find_stmt_, key};
    if (sqlite3_step(find_stmt_) != SQLITE_ROW)
        return Value{};
    auto result = sql_fetch_result<Value>(find_stmt_, 0);
    if (sqlite3_step(find_stmt_) != SQLITE_DONE)
        throw sqlite_map_exception{"find produced too much data: " + common::to_string(key)};
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

template <class Key, class Value>
void sqlite_map<Key, Value>::increment(const Key & key, const Value & amount)
{
    // if the key already exists, nothing happens from this insert
    // also, note that you should only increment integral/floating point types
    insert(key, static_cast<Value>(0));

    std::string command = "update map set value = value + "
        + sql_text(amount) + " where id = " + sql_text(key) + ";";

    std::lock_guard<std::mutex> lock{_mutex};
    _size_dirty = true;
    _commands += command;
    if(_num_cached++ >= _max_cached)
    {
        commit();
        _num_cached = 0;
    }
}

template <class Key, class Value>
uint64_t sqlite_map<Key, Value>::size() const
{
    /*
    std::lock_guard<std::mutex> lock{_mutex};

    if(!_size_dirty)
        return _size;

    commit();

    std::string command = "select count(*) from map;";
    if(sqlite3_exec(_db, command.c_str(), size_callback,
            (void*) this, nullptr) != SQLITE_OK)
        throw sqlite_map_exception{"error running command: " + command};

    _size_dirty = false;

    return _size;
    */
    std::lock_guard<std::mutex> lock{_mutex};
    sqlite_binder{size_stmt_};
    if (sqlite3_step(size_stmt_) != SQLITE_ROW)
        throw sqlite_map_exception{"failed to get size"};
    uint64_t size = sqlite3_column_int64(size_stmt_, 0);
    if (sqlite3_step(size_stmt_) != SQLITE_DONE)
        throw sqlite_map_exception{"size returned too much data"};
    return size;
}

template <class Key, class Value>
int sqlite_map<Key, Value>::size_callback(void* state, int num_cols,
        char** fields, char** cols)
{
    sqlite_map<Key, Value>* obj = static_cast<sqlite_map<Key, Value>*>(state);
    std::stringstream ss{fields[0]};
    ss >> obj->_size;

    return 0;
}

template <class Key, class Value>
int sqlite_map<Key, Value>::find_callback(void* state, int num_cols,
        char** fields, char** cols)
{
    sqlite_map<Key, Value>* obj = static_cast<sqlite_map<Key, Value>*>(state);
    std::stringstream ss{fields[0]};
    ss >> obj->_return_value;
    return 0;
}

}
}
