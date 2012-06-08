#include <iostream>

#include "compressed_file_reader.h"
#include "compressed_file_writer.h"

using std::cout;
using std::endl;

int main(int argc, char* argv[])
{
    CompressedFileReader reader("compressed.txt");

    while(reader.hasNext())
    {
        cout << reader.next();
        cout << endl;
    }

    return 0;
}
