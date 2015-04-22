/**
 * @file chunk.tcc
 * @author Sean Massung
 */

#include "index/chunk.h"
#include "index/postings_data.h"

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

    std::ifstream my_data{path_, std::ios::binary};
    std::ifstream other_data{other.path_, std::ios::binary};
    std::ofstream output{temp_name, std::ios::binary};

    postings_data<PrimaryKey, SecondaryKey> my_pd;
    postings_data<PrimaryKey, SecondaryKey> other_pd;
    my_pd.read_packed(my_data);
    other_pd.read_packed(other_data);

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
            my_pd.write_packed(output);

            // read next two postings data
            my_pd.read_packed(my_data);
            other_pd.read_packed(other_data);
        }
        else if (my_pd.primary_key() < other_pd.primary_key())
        {
            // write the winner
            my_pd.write_packed(output);
            // read next from the current chunk
            my_pd.read_packed(my_data);
        }
        else
        {
            // write the winner
            other_pd.write_packed(output);
            // read next from the other chunk
            other_pd.read_packed(other_data);
        }
    }

    // finish merging when one runs out
    while (my_data)
    {
        ++terms;
        my_pd.write_packed(output);
        my_pd.read_packed(my_data);
    }
    while (other_data)
    {
        ++terms;
        other_pd.write_packed(output);
        other_pd.read_packed(other_data);
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

    std::ifstream my_data{path_, std::ios::binary};
    std::ofstream output{temp_name, std::ios::binary};

    postings_data<PrimaryKey, SecondaryKey> my_pd;
    my_pd.read_packed(my_data);
    auto other_pd = pdata.begin();

    uint64_t terms = 0;
    while (my_data && other_pd != pdata.end())
    {
        ++terms;
        if (my_pd.primary_key() == other_pd->primary_key())
        {
            my_pd.merge_with(*other_pd);
            my_pd.write_packed(output);
            my_pd.read_packed(my_data);
            ++other_pd;
        }
        else if (my_pd.primary_key() < other_pd->primary_key())
        {
            my_pd.write_packed(output);
            my_pd.read_packed(my_data);
        }
        else
        {
            other_pd->write_packed(output);
            ++other_pd;
        }
    }

    // finish merging when one runs out
    while (my_data)
    {
        ++terms;
        my_pd.write_packed(output);
        my_pd.read_packed(my_data);
    }
    while (other_pd != pdata.end())
    {
        ++terms;
        other_pd->write_packed(output);
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
