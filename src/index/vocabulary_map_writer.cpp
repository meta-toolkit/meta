/**
 * @file vocabulary_map_writer.cpp
 * @author Chase Geigle
 */

#include <iostream>
#include <algorithm>
#include "meta/meta.h"
#include "meta/index/vocabulary_map_writer.h"
#include "meta/io/binary.h"

namespace meta
{
namespace index
{

vocabulary_map_writer::vocabulary_map_writer(const std::string& path,
                                             uint16_t block_size)
    : file_write_pos_{0},
      inverse_file_{path + ".inverse", std::ios::binary},
      path_{path},
      block_size_{block_size},
      num_terms_{0},
      remaining_block_space_{block_size},
      written_nodes_{0}
{
    file_.open(path, file_.binary | file_.trunc);
    if (!file_ || !inverse_file_)
        throw vocabulary_map_writer_exception{
            "failed to open vocabulary map file"};
}

void vocabulary_map_writer::insert(const std::string& term)
{
    if (term.empty())
        throw vocabulary_map_writer_exception{
            "empty string cannot be inserted into the vocabulary_map"};
    // + 1 for null terminator
    auto length = sizeof(term_id) + term.length() + 1;

    if (length > remaining_block_space_)
    {
        write_padding();
        ++written_nodes_;
    }
    // record term position in inverse file
    io::write_binary(inverse_file_, file_write_pos_);

    // write term and id to tree file
    io::write_binary(file_, term);
    io::write_binary(file_, num_terms_);

    // update write cursor (can't use fstream tell functions because these
    // files may be larger than 2GB)
    file_write_pos_ += length;

    remaining_block_space_ -= length;
    ++num_terms_;
}

void vocabulary_map_writer::write_padding()
{
    if (remaining_block_space_ > 0)
    {
        // -1 for null terminator
        std::string padding(remaining_block_space_ - 1, '\0');
        io::write_binary(file_, padding);
    }
    file_write_pos_ += remaining_block_space_;
    remaining_block_space_ = block_size_;
}

void vocabulary_map_writer::flush()
{
    write_padding();
    file_.flush();
    ++written_nodes_;
}

vocabulary_map_writer::~vocabulary_map_writer()
{
    // flush any remaining partial block
    if (remaining_block_space_ != block_size_)
        flush();
    file_.flush();

    uint64_t remaining_nodes{0};
    std::swap(remaining_nodes, written_nodes_);

    // begin writing the internal nodes
    std::ifstream reader{path_, std::ios::binary};
    uint64_t read_pos = 0;

    // this first loop is over each of the levels: when we have written
    // only one node inside the inner loop, we've just finished writing the
    // root and we can stop
    while (remaining_nodes != 1)
    {
        // this loop is over the last level that was written---once we have
        // processed all nodes in the previous level, we can move to the
        // next level's iteration
        while (remaining_nodes > 0)
        {
            uint64_t t_id;
            std::string term;
            io::read_binary(reader, term);
            io::read_binary(reader, t_id);

            // signed int for safe negation below (auto causes a big on
            // 32-bit systems)
            auto length
                = static_cast<int64_t>(sizeof(uint64_t) + term.length() + 1);

            // if we are out of room in the current block, flush it and start a
            // new one
            if (length > remaining_block_space_)
            {
                flush();
                // we need to re-read the block head we just read
                reader.seekg(-length, file_.cur);
            }
            // otherwise, write the head term of the block we are reading to
            // the current block along with its position
            else
            {
                // write the position of the block and the term at its head
                io::write_binary(file_, term);
                io::write_binary(file_, read_pos);

                // skip to the next block
                reader.seekg(block_size_ - length, file_.cur);
                read_pos += block_size_;

                // update the remaining space in the current block
                remaining_block_space_ -= length;
                --remaining_nodes;
            }
        }

        // if there is still a partial block remaining to be written for
        // this level, write it
        if (remaining_block_space_ != block_size_)
            flush();

        std::swap(remaining_nodes, written_nodes_);
    }
}
}
}
