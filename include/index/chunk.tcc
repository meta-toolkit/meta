/**
 * @file chunk.tcc
 * @author Sean Massung
 */

#include <fstream>
#include "util/filesystem.h"
#include "index/chunk.h"
#include "index/postings_data.h"

namespace meta {
namespace index {

template <class PrimaryKey, class SecondaryKey>
chunk<PrimaryKey, SecondaryKey>::chunk(const std::string & path):
    _path{path}
{
    set_size();
}

template <class PrimaryKey, class SecondaryKey>
void chunk<PrimaryKey, SecondaryKey>::set_size()
{
    _size = filesystem::file_size(_path);
}

template <class PrimaryKey, class SecondaryKey>
bool chunk<PrimaryKey, SecondaryKey>::operator<(const chunk & other) const
{
    // merge smaller chunks first
    return size() > other.size();
}

template <class PrimaryKey, class SecondaryKey>
std::string chunk<PrimaryKey, SecondaryKey>::path() const
{
    return _path;
}

template <class PrimaryKey, class SecondaryKey>
uint64_t chunk<PrimaryKey, SecondaryKey>::size() const
{
    return _size;
}

template <class PrimaryKey, class SecondaryKey>
void chunk<PrimaryKey, SecondaryKey>::merge_with(const chunk & other)
{
    std::string temp_name = _path + "_merge";

    io::compressed_file_reader my_data{_path,
        common::default_compression_reader_func};
    io::compressed_file_reader other_data{other._path,
        common::default_compression_reader_func};
    io::compressed_file_writer output{temp_name,
        common::default_compression_writer_func};

    postings_data<PrimaryKey, SecondaryKey> my_pd;
    postings_data<PrimaryKey, SecondaryKey> other_pd;
    my_data >> my_pd;
    other_data >> other_pd;

    uint64_t terms = 0;
    // merge while both have postings data
    while (my_data && other_data) {
        ++terms;
        if (my_pd.primary_key() == other_pd.primary_key()) {
            // merge
            my_pd.merge_with(other_pd);
            // write
            output << my_pd;

            // read next two postings data
            my_data >> my_pd;
            other_data >> other_pd;
        } else if (my_pd.primary_key() < other_pd.primary_key()) {
            // write the winner
            output << my_pd;
            // read next from the current chunk
            my_data >> my_pd;
        } else {
            // write the winner
            output << other_pd;
            // read next from the other chunk
            other_data >> other_pd;
        }
    }

    // finish merging when one runs out
    while (my_data) {
        ++terms;
        output << my_pd;
        my_data >> my_pd;
    }
    while (other_data) {
        ++terms;
        output << other_pd;
        other_data >> other_pd;
    }

    my_data.close();
    other_data.close();
    output.close();
    filesystem::delete_file(_path);
    filesystem::delete_file(_path + ".numterms");
    filesystem::delete_file(other._path);
    filesystem::delete_file(other._path + ".numterms");
    filesystem::rename_file(temp_name, _path);

    std::ofstream termfile{_path + ".numterms"};
    termfile << terms;
    set_size();
}

template <class PrimaryKey, class SecondaryKey>
template <class Container>
void chunk<PrimaryKey, SecondaryKey>::memory_merge_with(Container & pdata)
{
    std::string temp_name = _path + "_merge";

    io::compressed_file_reader my_data{_path,
        common::default_compression_reader_func};
    io::compressed_file_writer output{temp_name,
        common::default_compression_writer_func};

    postings_data<PrimaryKey, SecondaryKey> my_pd;
    my_data >> my_pd;
    auto other_pd = pdata.begin();

    uint64_t terms = 0;
    while (my_data && other_pd != pdata.end())
    {
        ++terms;
        if(my_pd.primary_key() == other_pd->primary_key())
        {
            my_pd.merge_with(*other_pd);
            output << my_pd;
            my_data >> my_pd;
            ++other_pd;
        }
        else if(my_pd.primary_key() < other_pd->primary_key())
        {
            output << my_pd;
            my_data >> my_pd;
        }
        else
        {
            output << *other_pd;
            ++other_pd;
        }
    }

    // finish merging when one runs out
    while(my_data)
    {
        ++terms;
        output << my_pd;
        my_data >> my_pd;
    }
    while(other_pd != pdata.end())
    {
        ++terms;
        output << *other_pd;
        ++other_pd;
    } 

    my_data.close();
    output.close();
    filesystem::delete_file(_path);
    filesystem::delete_file(_path + ".numterms");
    filesystem::rename_file(temp_name, _path);
    pdata.clear();

    std::ofstream termfile{_path + ".numterms"};
    termfile << terms;
    set_size();
}

}
}
