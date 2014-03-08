#include <fstream>
#include <iostream>

#include "utf/utf.h"

using namespace meta;

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " file charset" << std::endl;
        return 1;
    }

    std::ifstream file{argv[1]};
    std::string content;
    file.seekg(0, std::ios::end);
    content.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());

    auto u16 = utf::to_utf16(content, argv[2]);
    auto u8 = utf::to_utf8(u16);

    std::cout << u8 << std::endl;
}
