/**
 * @file compressor_test.cpp
 * @author Chase Geigle
 */

#include <array>
#include <iostream>
#include <fstream>
#include "meta/io/gzstream.h"

using namespace meta;

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " input output" << std::endl;
        return 1;
    }

    std::array<char, 1024> buffer;

    {
        std::ifstream file{argv[1], std::ios::in | std::ios::binary};
        io::gzofstream output{argv[2]};
        while (file)
        {
            file.read(&buffer[0], 1024);
            output.write(&buffer[0], file.gcount());
        }
    }

    {
        io::gzifstream input{argv[2]};
        std::ofstream output{std::string{argv[2]} + ".decompressed",
                             std::ios::out | std::ios::binary};

        while (input)
        {
            input.read(&buffer[0], 1024);
            output.write(&buffer[0], input.gcount());
        }
    }

    return 0;
}
