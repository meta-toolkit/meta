/**
 * @file compressed_file_writer.cpp
 */

#include "compressed_file_writer.h"

using std::cerr;
using std::endl;
using std::string;

CompressedFileWriter::CompressedFileWriter(const string & filename)
{
    _outfile = fopen(filename.c_str(), "w");
    _charCursor = 0;
    _bitCursor = 0;
    _bufferSize = 1024 * 1024 * 8,
    _buffer = new unsigned char[_bufferSize];

    // disable buffering
    if(setvbuf(_outfile, NULL, _IONBF, 0) != 0)
        cerr << "[CompressedFileWriter]: error setvbuf" << endl;

    // zero out, we'll only write ones
    memset(_buffer, 0, _bufferSize);
}

CompressedFileWriter::~CompressedFileWriter()
{
    // write the remaining bits, up to the nearest byte
    fwrite(_buffer, 1, _charCursor + 1, _outfile);
    delete [] _buffer;
    fclose(_outfile);
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
    if(bit)
        _buffer[_charCursor] |= (1 << (7 - _bitCursor));

    if(++_bitCursor == 8)
    {
        _bitCursor = 0;
        if(++_charCursor == _bufferSize)
        {
            _charCursor = 0;
            writeBuffer();
        }
    }
}

void CompressedFileWriter::writeBuffer() const
{
    if(fwrite(_buffer, 1, _bufferSize, _outfile) != _bufferSize)
        cerr << "[CompressedFileWriter]: error writing to file" << endl;
    memset(_buffer, 0, _bufferSize);
}
