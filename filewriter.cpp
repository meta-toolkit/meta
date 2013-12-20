/**
 * @file filewriter.cpp
 */

#include <string>
#include <iostream>
#include "io/file_writer.h"
#include "io/file_reader.h"

using namespace meta;

void test_write()
{
    io::file_writer fw{"output.txt"};
    fw.write(std::string{"test"});
    fw.write(std::string{"testing"});
    fw.write<int>(7);
    fw.write<int>(47);
}

void test_read()
{
    io::file_reader fr{"output.txt"};
    std::string s1, s2;
    fr.read(s1);
    fr.read(s2);
    std::cout << s1 << std::endl;
    std::cout << s2 << std::endl;
    int a, b;
    fr.read(a);
    fr.read(b);
    std::cout << a << std::endl;
    std::cout << b << std::endl;
}

int main()
{
    std::cout << "testing read" << std::endl;
    test_write();
    std::cout << "testing write" << std::endl;
    test_read();
}
