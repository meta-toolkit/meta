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
    sqlite3_close(_db);
}

template <class Key, class Value>
void sqlite_map<Key, Value>::insert(const Key & key, const Value & value)
{
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
    std::string command = "select value from map where id = ";
    command += sql_text(key);

    std::lock_guard<std::mutex> lock{_mutex};
    commit();

    if(sqlite3_exec(_db, command.c_str(), find_callback,
            (void*) this, nullptr) != SQLITE_OK)
        throw sqlite_map_exception{"error running command: " + command};

    return _return_value; 
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
uint64_t sqlite_map<Key, Value>::size()
{
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
