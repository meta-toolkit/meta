#include <string>
#include <iostream>
#include "parse_tree.h"

using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[])
{
    ParseTree tree("(S(NP)(VP(AB)(BC)))");
    cout << tree.getString() << endl;
    return 0;
}
