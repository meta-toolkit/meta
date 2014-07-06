/**
 * @file lm-test.cpp
 * @author Sean Massung
 */

#include "meta.h"
#include "lm/language_model.h"

using namespace meta;

int main(int argc, char* argv[])
{
    lm::language_model<3> model{argv[1]};
    for(size_t i = 10; i < 40; ++i)
        std::cout << model.generate(i) << std::endl;
}
