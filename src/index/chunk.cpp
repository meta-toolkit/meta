/**
 * @file chunk.cpp
 * @author Sean Massung
 */

#include <sys/stat.h>
#include <cstdio>
#include <fstream>
#include "index/chunk.h"
#include "index/postings_data.h"

namespace meta {
namespace index {

chunk::chunk(const std::string & path):
    _path(path)
{
    set_size();
}

void chunk::set_size()
{
    struct stat st;
    stat(_path.c_str(), &st);
    _size = st.st_size;
}

bool chunk::operator<(const chunk & other) const
{
    // merge smaller chunks first
    return size() > other.size();
}

std::string chunk::path() const
{
    return _path;
}

uint64_t chunk::size() const
{
    return _size;
}

void chunk::merge_with(const chunk & other)
{
    std::ifstream my_data{_path.c_str()};
    std::ifstream other_data{other._path.c_str()};
    std::ofstream output{"chunk-temp"};

    // TODO how to instantiate for both index types?
    postings_data<term_id, doc_id> my_pd{0};
    postings_data<term_id, doc_id> other_pd{0};
    my_data >> my_pd;
    other_data >> other_pd;

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

    postings_data<term_id, doc_id> buffer{0}; // TODO same here
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
    rename("chunk-temp", _path.c_str());

    set_size();
}

}
}
