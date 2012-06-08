/**
 * @file compressed_file_writer.cpp
 */

#include "compressed_file_writer.h"

CompressedFileWriter::CompressedFileWriter(const string & filename)
{

}

void CompressedFileWriter::write(unsigned int value)
{
    int length = log2(value);

    for(int bit = 0; bit < length; ++bit)
        writeBit(false);

    writeBit(true);

    for(int bit = length - 1; bit >= 0; --bit)
        writeBit(value & 1 << bit);
}

void CompressedFileWriter::writeBit(bool bit)
{

}
