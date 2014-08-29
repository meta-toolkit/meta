#include <memory>
#include "util/optional.h"
#include "index/vocabulary_map.h"

int main(int argc, char** argv)
{
    using namespace meta;
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " filename term" << std::endl;
        return 1;
    }

    index::vocabulary_map map{argv[1]};

    std::cout << *map.find(argv[2]) << std::endl;
}
