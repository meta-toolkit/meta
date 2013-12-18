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
    _path{path}
{
    set_size();
}

template <class PrimaryKey, class SecondaryKey>
void chunk<PrimaryKey, SecondaryKey>::set_size()
{
    _size = common::file_size(_path);
    std::ifstream data{_path};
    common::read_binary(data, _terms);
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

    std::string temp_name = _path + "_merge";
    std::ofstream output{temp_name};

    postings_data<PrimaryKey, SecondaryKey> my_pd;
    postings_data<PrimaryKey, SecondaryKey> other_pd;

    // skip the term count at the beginning of the files
    uint64_t terms;
    common::read_binary(my_data, terms);
    common::read_binary(other_data, terms);
    common::write_binary(output, terms); // dummy output for now
                                         // until know the true count

    my_data >> my_pd;
    other_data >> other_pd;

    _terms = 0;
    // merge while both have postings data
    while (my_data && other_data) {
        ++_terms;
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
        ++_terms;
        output << my_pd;
        my_data >> my_pd;
    }
    while (other_data) {
        ++_terms;
        output << other_pd;
        other_data >> other_pd;
    }

    my_data.close();
    other_data.close();
    output.seekp(0, output.beg);
    common::write_binary(output, _terms);
    output.close();
    remove(_path.c_str());
    remove(other._path.c_str());
    rename(temp_name.c_str(), _path.c_str());

    set_size();
}

}
}
