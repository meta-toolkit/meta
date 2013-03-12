/**
 * @file textfile.cpp
 */

#include "io/textfile.h"

using std::string;

TextFile::TextFile(string path):
    _path(path), _start(nullptr), _file_descriptor(-1)
{

    struct stat st;
    stat(_path.c_str(), &st);
    _size = st.st_size; 
   
    if(_size >= _min_mmap_size)
        open_mmap();
    else
        load_file();
}

void TextFile::load_file()
{
    _start = new char[_size];
    FILE* file = fopen(_path.c_str(), "r");
    fread(_start, _size, 1, file);
    fclose(file);
}

void TextFile::open_mmap()
{
    _file_descriptor = open(_path.c_str(), O_RDONLY);
    if(_file_descriptor < 0)
        throw TextFileException("error obtaining file descriptor for " + _path);

    _start = (char*) mmap(nullptr, _size, PROT_READ, MAP_SHARED, _file_descriptor, 0);
    if(_start == nullptr)
    {
        close(_file_descriptor);
        throw TextFileException("error memory-mapping " + _path);
    }
}

unsigned int TextFile::size() const
{
    return _size;
}

string TextFile::path() const
{
    return _path;
}

char* TextFile::start() const
{
    return _start;
}

TextFile::~TextFile()
{
    if(_size >= _min_mmap_size)
    {
        if(_start != nullptr)
        {
            munmap(_start, _size);
            close(_file_descriptor);
        }
    }
    else
    {
        delete [] _start;
    }
}
