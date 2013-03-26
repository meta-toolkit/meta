#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <iostream>

#include "index/document.h"
#include "io/config_reader.h"
#include "tokenizers/tokenizer.h"
#include "util/invertible_map.h"
#include "util/common.h"

#include "classify/select_slda.h"
#include "classify/select_doc_freq.h"
#include "classify/select_chi_square.h"
#include "classify/select_info_gain.h"

using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::pair;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cerr << "Usage:\t" << argv[0] << " config.ini" << endl;
        return 1;
    }

    unordered_map<string, string> config = ConfigReader::read(argv[1]);
    string prefix = "/home/sean/projects/senior-thesis-data/" + config["prefix"];
    vector<Document> documents = Document::loadDocs(prefix + "/full-corpus.txt", prefix);
    std::shared_ptr<Tokenizer> tokenizer = ConfigReader::create_tokenizer(config);

    InvertibleMap<string, int> mapping;
    size_t i = 0;
    for(auto & doc: documents)
    {
        tokenizer->tokenize(doc, nullptr);
        Common::show_progress(i++, documents.size(), 20, "  tokenizing ");
    }
    Common::end_progress("  tokenizing ");

    vector<pair<TermID, double>> features = classify::feature_select::info_gain(documents);
    for(size_t i = 0; i < features.size(); ++i)
        cout << " " << tokenizer->getLabel(features[i].first)
             << " " << features[i].second << endl;

    return 0;
}
