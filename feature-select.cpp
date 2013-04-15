#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <iostream>

#include "index/document.h"
#include "io/config_reader.h"
#include "tokenizers/ngram_tokenizer.h"
#include "tokenizers/tree_tokenizer.h"
#include "util/invertible_map.h"

#include "classify/select_doc_freq.h"
#include "classify/select_chi_square.h"
#include "classify/select_info_gain.h"
#include "classify/select_corr.h"
#include "classify/select_odds.h"

using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::pair;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ofstream;

using namespace meta;
using namespace meta::index;
using namespace meta::util;
using namespace meta::tokenizers;

int main(int argc, char8 argv[])
{
    return 0;
}
