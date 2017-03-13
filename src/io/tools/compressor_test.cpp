/**
 * @file compressor_test.cpp
 * @author Chase Geigle
 */

#include "meta/io/gzstream.h"
#include <array>
#include <fstream>
#include <iostream>
#if META_HAS_LIBLZMA
#include "meta/io/xzstream.h"
#endif

using namespace meta;

template <class InputStream, class OutputStream>
void test_compressor(const std::string& infile, const std::string& outfile)
{
    std::array<char, 1024> buffer;

    {
        std::ifstream file{infile, std::ios::in | std::ios::binary};
        OutputStream output{outfile};
        while (file)
        {
            file.read(&buffer[0], 1024);
            output.write(&buffer[0], file.gcount());
        }
    }

    {
        InputStream input{outfile};
        std::ofstream output{outfile + ".decompressed",
                             std::ios::out | std::ios::binary};

        while (input)
        {
            input.read(&buffer[0], 1024);
            output.write(&buffer[0], input.gcount());
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " input output" << std::endl;
        return 1;
    }


    test_compressor<io::gzifstream, io::gzofstream>(argv[1], argv[2]);
#if META_HAS_LIBLZMA
    test_compressor<io::xzifstream, io::xzofstream>(
        argv[1], std::string{argv[2]} + ".xz");
#endif

    return 0;
}
