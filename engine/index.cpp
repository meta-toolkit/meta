#include <string>
#include <iostream>
#include "parse_tree.h"

using std::cout;
using std::endl;
using std::string;

int main(int argc, char* argv[])
{
    vector<string> filenames;


    string s = "(S(NP)(VP(AB)(BC)))";
    ParseTree tree(s);
    cout << tree.getString() << endl;
    return 0;
}
