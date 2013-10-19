/**
 * @file chunk.tcc
 * @author Sean Massung
 */

#include <sys/stat.h>
#include <cstdio>
#include <fstream>
#include "index/chunk.h"
#include "index/postings_data.h"

namespace meta {
namespace index {

template <class PrimaryKey, class SecondaryKey>
chunk<PrimaryKey, SecondaryKey>::chunk(const std::string & path):
    _path(path)
{
    set_size();
}

template <class PrimaryKey, class SecondaryKey>
void chunk<PrimaryKey, SecondaryKey>::set_size()
{
    _size = common::file_size(_path);
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
    std::ifstream my_data{_path};
    std::ifstream other_data{other._path};

    std::string temp_name = _path + "_" + other._path;
    std::ofstream output{temp_name};

    postings_data<PrimaryKey, SecondaryKey> my_pd{PrimaryKey{0}};
    postings_data<PrimaryKey, SecondaryKey> other_pd{PrimaryKey{0}};
    my_data >> my_pd;
    other_data >> other_pd;

    // merge while both have postings data

    while(true)
    {
        if(my_pd.primary_key() == other_pd.primary_key())
        {
            my_pd.merge_with(other_pd);
            output << my_pd;
            if(my_data.good())
                my_data >> my_pd;
            if(other_data.good())
                other_data >> other_pd;
            if(!my_data.good() || !other_data.good())
                break;
        }
        else if(my_pd < other_pd)
        {
            output << my_pd;
            if(my_data.good())
                my_data >> my_pd;
            else
                break;
        }
        else
        {
            output << other_pd;
            if(other_data.good())
                other_data >> other_pd;
            else
                break;
        }
    }

    // finish merging when one runs out

    postings_data<PrimaryKey, SecondaryKey> buffer{PrimaryKey{0}};
    if(my_data.good())
    {
        output << my_pd;
        while(my_data.good())
        {
            my_data >> buffer;
            output << buffer;
        }
    }
    else if(other_data.good())
    {
        output << other_pd;
        while(other_data.good())
        {
            other_data >> buffer;
            output << buffer;
        }
    }

    my_data.close();
    other_data.close();
    output.close();
    remove(_path.c_str());
    remove(other._path.c_str());
    rename(temp_name.c_str(), _path.c_str());

    set_size();
}

}
}
