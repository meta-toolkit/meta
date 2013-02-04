/**
 * @file textfile.cpp
 */

#include "textfile.h"

using std::cerr;
using std::endl;
using std::string;

TextFile::TextFile(string path):
    _path(path), _start(NULL), _file_descriptor(-1)
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
    {
        cerr << "[TextFile]: Error obtaining file descriptor for: " << _path.c_str() << endl;
        return;
    }

    _start = (char*) mmap(NULL, _size, PROT_READ, MAP_SHARED, _file_descriptor, 0);
    if(_start == NULL)
    {
        cerr << "[TextFile]: Error memory-mapping " << _path << endl;
        close(_file_descriptor);
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
        if(_start != NULL)
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
