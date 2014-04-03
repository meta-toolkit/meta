/**
 * @file chunk.tcc
 * @author Sean Massung
 */

#include "index/chunk.h"
#include "index/postings_data.h"

#include "io/compressed_file_reader.h"
#include "io/compressed_file_writer.h"
#include "util/filesystem.h"

namespace meta
{
namespace index
{

template <class PrimaryKey, class SecondaryKey>
chunk<PrimaryKey, SecondaryKey>::chunk(const std::string& path)
    : path_{path}
{
    set_size();
}

template <class PrimaryKey, class SecondaryKey>
void chunk<PrimaryKey, SecondaryKey>::set_size()
{
    size_ = filesystem::file_size(path_);
}

template <class PrimaryKey, class SecondaryKey>
bool chunk<PrimaryKey, SecondaryKey>::operator<(const chunk& other) const
{
    // merge smaller chunks first
    return size() > other.size();
}

template <class PrimaryKey, class SecondaryKey>
std::string chunk<PrimaryKey, SecondaryKey>::path() const
{
    return path_;
}

template <class PrimaryKey, class SecondaryKey>
uint64_t chunk<PrimaryKey, SecondaryKey>::size() const
{
    return size_;
}

template <class PrimaryKey, class SecondaryKey>
void chunk<PrimaryKey, SecondaryKey>::merge_with(const chunk& other)
{
    std::string temp_name = path_ + "_merge";

    io::compressed_file_reader my_data{path_,
                                       io::default_compression_reader_func};
    io::compressed_file_reader other_data{other.path_,
                                          io::default_compression_reader_func};
    io::compressed_file_writer output{temp_name,
                                      io::default_compression_writer_func};

    postings_data<PrimaryKey, SecondaryKey> my_pd;
    postings_data<PrimaryKey, SecondaryKey> other_pd;
    my_data >> my_pd;
    other_data >> other_pd;

    uint64_t terms = 0;
    // merge while both have postings data
    while (my_data && other_data)
    {
        ++terms;
        if (my_pd.primary_key() == other_pd.primary_key())
        {
            // merge
            my_pd.merge_with(other_pd);
            // write
            output << my_pd;

            // read next two postings data
            my_data >> my_pd;
            other_data >> other_pd;
        }
        else if (my_pd.primary_key() < other_pd.primary_key())
        {
            // write the winner
            output << my_pd;
            // read next from the current chunk
            my_data >> my_pd;
        }
        else
        {
            // write the winner
            output << other_pd;
            // read next from the other chunk
            other_data >> other_pd;
        }
    }

    // finish merging when one runs out
    while (my_data)
    {
        ++terms;
        output << my_pd;
        my_data >> my_pd;
    }
    while (other_data)
    {
        ++terms;
        output << other_pd;
        other_data >> other_pd;
    }

    my_data.close();
    other_data.close();
    output.close();
    filesystem::delete_file(path_);
    filesystem::delete_file(path_ + ".numterms");
    filesystem::delete_file(other.path_);
    filesystem::delete_file(other.path_ + ".numterms");
    filesystem::rename_file(temp_name, path_);

    std::ofstream termfile{path_ + ".numterms"};
    termfile << terms;
    set_size();
}

template <class PrimaryKey, class SecondaryKey>
template <class Container>
void chunk<PrimaryKey, SecondaryKey>::memory_merge_with(Container& pdata)
{
    std::string temp_name = path_ + "_merge";

    io::compressed_file_reader my_data{path_,
                                       io::default_compression_reader_func};
    io::compressed_file_writer output{temp_name,
                                      io::default_compression_writer_func};

    postings_data<PrimaryKey, SecondaryKey> my_pd;
    my_data >> my_pd;
    auto other_pd = pdata.begin();

    uint64_t terms = 0;
    while (my_data && other_pd != pdata.end())
    {
        ++terms;
        if (my_pd.primary_key() == other_pd->primary_key())
        {
            my_pd.merge_with(*other_pd);
            output << my_pd;
            my_data >> my_pd;
            ++other_pd;
        }
        else if (my_pd.primary_key() < other_pd->primary_key())
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
    while (my_data)
    {
        ++terms;
        output << my_pd;
        my_data >> my_pd;
    }
    while (other_pd != pdata.end())
    {
        ++terms;
        output << *other_pd;
        ++other_pd;
    }

    my_data.close();
    output.close();
    filesystem::delete_file(path_);
    filesystem::delete_file(path_ + ".numterms");
    filesystem::rename_file(temp_name, path_);
    pdata.clear();

    std::ofstream termfile{path_ + ".numterms"};
    termfile << terms;
    set_size();
}
}
}
