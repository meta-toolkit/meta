/**
 * @file vocabulary_map_writer.h
 * @author Chase Geigle
 */

#ifndef _META_VOCABULARY_MAP_WRITER_H_
#define _META_VOCABULARY_MAP_WRITER_H_

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>

namespace meta
{
namespace index
{

class vocabulary_map_writer
{
  private:
    std::ofstream file_;
    uint64_t file_write_pos_;
    std::ofstream inverse_file_;
    std::string path_;
    uint16_t block_size_;
    uint64_t num_terms_;
    uint16_t remaining_block_space_;
    uint64_t written_nodes_;

  public:
    vocabulary_map_writer(const std::string& path, uint16_t block_size = 4096);
    ~vocabulary_map_writer();

    void insert(const std::string& term);

    class vocabulary_map_writer_exception: public std::runtime_error
    {
      using std::runtime_error::runtime_error;
    };

  private:
    void write_padding();
    void flush();
};
}
}
#endif
