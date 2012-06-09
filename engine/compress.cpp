#include <iostream>

#include "compressed_file_reader.h"
#include "compressed_file_writer.h"

using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
    CompressedFileReader reader("in-compressed.txt");
    CompressedFileWriter writer("out-compressed.txt");

    while(reader.hasNext())
    {
        unsigned int next = reader.next();
        cout << next;
        cout << endl;
        writer.write(next);
    }

    return 0;
}
