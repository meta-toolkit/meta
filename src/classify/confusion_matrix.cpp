/**
 * @file confusion_matrix.cpp
 * @author Sean Massung
 */

#include <iomanip>
#include "util/common.h"
#include "classify/confusion_matrix.h"

namespace meta {
namespace classify {

using std::setw;
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
    _counts(unordered_map<string, size_t>()),
    _total(0)
{ /* nothing */ }

void ConfusionMatrix::add(const string & predicted, const string & actual)
{
    pair<string, string> prediction(predicted, actual);
    _predictions[prediction] += 1;
    _counts[actual] += 1;
    _classes.insert(actual);
    ++_total;
}

void ConfusionMatrix::print(std::ostream & out) const
{
    size_t w = 12;
    out << endl << setw(w) << "";
    for(auto & aClass: _classes)
        out << setw(w - 1) << aClass << " ";
    out << endl;
    out << string(w, ' ') << string(_classes.size() * w, '-') << endl;

    for(auto & aClass: _classes)
    {
        out << setw(w) << (aClass + " | ");
        for(auto & pClass: _classes)
        {
            auto predIt = _predictions.find(make_pair(pClass, aClass));
            if(predIt != _predictions.end())
            {
                size_t numPred = predIt->second;
                double percent = static_cast<double>(numPred) / _counts.at(aClass);
                out.precision(3);
                if(aClass == pClass)
                    out << "[" << setw(w - 2) << percent << "]";
                else
                    out << setw(w) << percent;
            }
            else
                out << setw(w) << "- ";
        }
        out << endl;
    }
    out << endl;
}

void ConfusionMatrix::print_class_stats(std::ostream & out, const string & label,
        double & prec, double & rec, double & f1) const
{
    for(auto & cls: _classes)
    {
        prec += common::safe_at(_predictions, make_pair(cls, label));
        rec +=  common::safe_at(_predictions, make_pair(label, cls));
    }

    double correct = common::safe_at(_predictions, make_pair(label, label));
    
    if(rec != 0.0)
        rec = correct / rec;

    if(prec != 0.0)
        prec = correct / prec;

    if(prec + rec != 0.0)
        f1 = (2.0 * prec * rec) / (prec + rec);

    size_t w = 20;
    out << std::left << setw(w) << label
        << std::left << setw(w) << f1
        << std::left << setw(w) << prec
        << std::left << setw(w) << rec
        << endl;
}

void ConfusionMatrix::print_stats(std::ostream & out) const
{
    double t_prec = 0.0;
    double t_rec = 0.0;
    double t_f1 = 0.0;
    double t_corr = 0.0;

    size_t w = 20;
    out.precision(3);
    out << string(w * 4, '-') << endl
        << std::left << setw(w) << "Class"
        << std::left << setw(w) << "F1 Score"
        << std::left << setw(w) << "Precision"
        << std::left << setw(w) << "Recall" << endl
        << string(w * 4, '-') << endl;

    for(auto & cls: _classes)
    {
        double prec = 0.0, rec = 0.0, f1 = 0.0;
        auto it = _predictions.find(make_pair(cls, cls));
        if(it != _predictions.end())
            t_corr += it->second;
        print_class_stats(out, cls, prec, rec, f1);
        t_prec += prec;
        t_rec += rec;
        t_f1 += f1;
    }

    out << string(w * 4, '-') << endl
        << setw(w) << "Total"
        << setw(w) << t_f1 / _classes.size()
        << setw(w) << t_prec / _classes.size()
        << setw(w) << t_rec / _classes.size() << endl
        << string(w * 4, '-') << endl
        << _total << " predictions attempted, overall accuracy: "
        << t_corr / _total << endl;
}

size_t ConfusionMatrix::stringPairHash(const std::pair<std::string, std::string> & str_pair)
{
    return std::hash<string>()(str_pair.first + str_pair.second);
}

}
}
