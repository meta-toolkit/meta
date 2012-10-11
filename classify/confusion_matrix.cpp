/**
 * @file confusion_matrix.cpp
 */

#include <iomanip>
#include <iostream>
#include "util/common.h"
#include "confusion_matrix.h"

using std::setw;
using std::cout;
using std::endl;
using std::unordered_map;
using std::unordered_set;
using std::string;
using std::pair;
using std::make_pair;

ConfusionMatrix::ConfusionMatrix():
    _predictions(unordered_map<pair<string, string>, size_t,
            decltype(&ConfusionMatrix::stringPairHash)>(32, stringPairHash)),
    _classes(unordered_set<string>()),
    _counts(unordered_map<string, size_t>()) { /* nothing */ }

void ConfusionMatrix::add(const string & predicted, const string & actual)
{
    pair<string, string> prediction(predicted, actual);
    auto it = _predictions.find(prediction);
    if(it == _predictions.end())
        _predictions.insert(make_pair(prediction, 1));
    else
        _predictions[prediction] += 1;

    auto act = _counts.find(actual);
    if(act == _counts.end())
        _counts.insert(make_pair(actual, 1));
    else
        _counts[actual] += 1;

    auto cl = _classes.find(actual);
    if(cl == _classes.end())
        _classes.insert(actual);
}

void ConfusionMatrix::print() const
{
    size_t w = 10;
    cout << endl << setw(w) << "";
    for(auto & aClass: _classes)
        cout << setw(w - 1) << aClass << " ";
    cout << endl;
    cout << string(w, ' ') << string(_classes.size() * w, '-') << endl;

    for(auto & aClass: _classes)
    {
        cout << setw(w) << (aClass + " | ");
        for(auto & pClass: _classes)
        {
            auto predIt = _predictions.find(make_pair(pClass, aClass));
            if(predIt != _predictions.end())
            {
                size_t numPred = predIt->second;
                double rounded = (int)((double) numPred / _counts.at(aClass) * 10000);
                string percent = Common::toString(rounded / 100);
                cout << setw(w) << ((aClass == pClass) ? ("[" + percent + "]") : (percent + " "));
            }
            else
                cout << setw(w) << "- ";
        }
        cout << endl;
    }
    cout << endl;
}

size_t ConfusionMatrix::stringPairHash(const std::pair<std::string, std::string> & strPair)
{
    return std::hash<string>()(strPair.first) ^ std::hash<string>()(strPair.second);
}
