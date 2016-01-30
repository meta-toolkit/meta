#include <iostream>
#include "meta/index/vocabulary_map.h"

using namespace meta;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " filename" << std::endl;
        return 1;
    }

    index::vocabulary_map map{argv[1]};

    for (term_id tid{0}; tid < map.size(); ++tid)
        std::cout << map.find_term(tid) << std::endl;
}
