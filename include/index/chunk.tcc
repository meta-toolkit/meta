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

namespace {

template <class ForwardIter1, class ForwardIter2, class OutputIter>
uint64_t merge_pdata(ForwardIter1 first, ForwardIter1 end1,
                     ForwardIter2 second, ForwardIter2 end2,
                     OutputIter output) {
    uint64_t num_unique = 0;
    while (first != end1 && second != end2) {
        auto & my_pd    = *first;
        auto & other_pd = *second;
        ++num_unique;
        if (my_pd.primary_key() == other_pd.primary_key()) {
            // merge
            auto out = std::move(my_pd);
            auto other = std::move(other_pd);
            out.merge_with(other);
            // write
            *output = out;

            // read next two postings data
            ++first;
            ++second;
        } else if (my_pd.primary_key() < other_pd.primary_key()) {
            // write the winner
            auto out = std::move(my_pd);
            *output = out;
            // read next from first chunk
            ++first;
        } else {
            // write the winner
            auto out = std::move(other_pd);
            *output = out;
            // read next from second chunk
            ++second;
        }
        ++output;
    }

    // finish merging when one runs out
    while (first != end1) {
        ++num_unique;
        *output = std::move(*first);
        ++output;
        ++first;
    }
    while (second != end2) {
        ++num_unique;
        *output = std::move(*second);
        ++output;
        ++second;
    }

    return num_unique;
}

}

template <class PrimaryKey, class SecondaryKey>
void chunk<PrimaryKey, SecondaryKey>::merge_with(const chunk & other)
{
    std::ifstream my_data{_path};
    std::ifstream other_data{other._path};

    std::string temp_name = _path + "_merge";
    std::ofstream output{temp_name};

    // skip the term count at the beginning of the files
    uint64_t terms;
    common::read_binary(my_data, terms);
    common::read_binary(other_data, terms);
    common::write_binary(output, terms); // dummy output for now until we
                                         // know the true count

    using pdata_t = postings_data<PrimaryKey, SecondaryKey>;
    using pdata_t_iterator = std::istream_iterator<pdata_t>;

    _terms = merge_pdata(pdata_t_iterator{my_data}, pdata_t_iterator{},
                         pdata_t_iterator{other_data}, pdata_t_iterator{},
                         std::ostream_iterator<pdata_t>{output});

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

template <class PrimaryKey, class SecondaryKey>
template <class Container>
void chunk<PrimaryKey, SecondaryKey>::memory_merge_with(Container & pdata)
{
    std::ifstream my_data{_path};
    std::string temp_name = _path + "_merge";
    std::ofstream output{temp_name};

    // skip the term count at the beginning of the files
    uint64_t terms;
    common::read_binary(my_data, terms);
    common::write_binary(output, terms); // dummy output for now until we
                                         // know the true count

    using pdata_t = postings_data<PrimaryKey, SecondaryKey>;
    using pdata_t_iterator = std::istream_iterator<pdata_t>;

    _terms = merge_pdata(pdata_t_iterator{my_data}, pdata_t_iterator{},
                         std::begin(pdata), std::end(pdata),
                         std::ostream_iterator<pdata_t>{output});

    my_data.close();
    output.seekp(0, output.beg);
    common::write_binary(output, _terms);
    output.close();
    remove(_path.c_str());
    rename(temp_name.c_str(), _path.c_str());
    pdata.clear();
    set_size();
}

}
}
