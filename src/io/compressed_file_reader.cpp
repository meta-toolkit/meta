/**
 * @file compressed_file_reader.cpp
 */

#include "io/compressed_file_reader.h"

using std::string;

CompressedFileReader::CompressedFileReader(const string & filename):
    _status(notDone), _currentValue(0), _currentChar(0), _currentBit(0)
{
    struct stat st;
    stat(filename.c_str(), &st);
    _size = st.st_size;
    
    // get file descriptor
    _fileDescriptor = open(filename.c_str(), O_RDONLY);
    if(_fileDescriptor < 0)
        throw CompressedFileReaderException("error obtaining file descriptor for " + filename);
    
    // memory map
    _start = (unsigned char*) mmap(nullptr, _size, PROT_READ, MAP_SHARED, _fileDescriptor, 0);
    if(_start == nullptr)
    {
        close(_fileDescriptor);
        throw CompressedFileReaderException("error memory-mapping the file");
    }

    // initialize the stream
    getNext();
}

CompressedFileReader::~CompressedFileReader()
{
    if(_start != nullptr)
    { 
        // unmap memory and close file
        munmap(_start, _size);
        close(_fileDescriptor);
    }
}

void CompressedFileReader::reset()
{
    _currentChar = 0;
    _currentBit = 0;
    _status = notDone;
    getNext();
}

void CompressedFileReader::seek(unsigned int position, unsigned int bitOffset)
{
    if(bitOffset <= 7 && position > _size)
    {
        _currentChar = position;
        _currentBit = bitOffset;
        _status = notDone;
        getNext();
    }
    else
        CompressedFileReaderException("error seeking; invalid parameters");
}

bool CompressedFileReader::hasNext() const
{
    return _status != readerDone;
}

unsigned int CompressedFileReader::next()
{
    if(_status == userDone)
        return 0;

    if(_status == readerDone)
    {
        _status = userDone;
        return _currentValue;
    }

    unsigned int next = _currentValue;
    getNext();
    return next;
}

void CompressedFileReader::getNext()
{
    int numberBits = 0;
    while(_status == 0 && !readBit())
        ++numberBits;

    _currentValue = 0;
    for(int bit = numberBits - 1; _status == 0 && bit >= 0; --bit)
    {
        if(readBit())
            _currentValue |= (1 << bit);
    }

    _currentValue |= (1 << numberBits);
}

bool CompressedFileReader::readBit()
{
    // (7 - _currentBit) to read from left to right
    bool bit = _start[_currentChar] & (1 << (7 - _currentBit));
    if(_currentBit == 7)
    {
        _currentBit = 0;
        if(++_currentChar == _size)
            _status = readerDone;
    }
    else
    {
        ++_currentBit;
    }
    return bit;
}
