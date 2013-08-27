/**
 * @file confusion_matrix.cpp
 * @author Sean Massung
 */

#include <iomanip>
#include <algorithm>
#include <vector>
#include "util/common.h"
#include "classify/confusion_matrix.h"

namespace meta {
namespace classify {

confusion_matrix::confusion_matrix():
    _total{0},
    _width{20}
{ /* nothing */ }

void confusion_matrix::add(const class_label & predicted, const class_label & actual, size_t times)
{
    std::pair<class_label, class_label> prediction{predicted, actual};
    _predictions[prediction] += times;
    _counts[actual] += times;
    _classes.insert(predicted);
    _classes.insert(actual);
    _total += times;
}

void confusion_matrix::print_result_pairs(std::ostream & out) const
{
    for(auto & p: _predictions)
    {
        for(size_t i = 0; i < p.second; ++i)
            out << p.first.first << " " << p.first.second << "\n";
    }
}

void confusion_matrix::print(std::ostream & out) const
{
    size_t w = 12;
    out << std::endl << std::setw(w) << "";
    for(auto & aClass: _classes)
        out << std::setw(w - 1) << aClass << " ";
    out << std::endl;
    out << std::string(w, ' ') << std::string(_classes.size() * w, '-')
        << std::endl;

    for(auto & aClass: _classes)
    {
        out << std::setw(w - 1) << aClass << " | ";
        for(auto & pClass: _classes)
        {
            auto predIt = _predictions.find(std::make_pair(pClass, aClass));
            if(predIt != _predictions.end())
            {
                size_t numPred = predIt->second;
                double percent = static_cast<double>(numPred) / _counts.at(aClass);
                out.precision(3);
                std::stringstream ss;
                ss.precision(3);
                ss << percent;
                if(aClass == pClass)
                    out << std::setw(w) << ("[" + ss.str() + "]");
                else
                    out << std::setw(w) << std::fixed << percent;
            }
            else
                out << std::setw(w) << "- ";
        }
        out << std::endl;
    }
    out << std::endl;
}

void confusion_matrix::print_class_stats(std::ostream & out, const class_label & label,
        double & prec, double & rec, double & f1) const
{
    for(auto & cls: _classes)
    {
        prec += common::safe_at(_predictions, std::make_pair(cls, label));
        rec  += common::safe_at(_predictions, std::make_pair(label, cls));
    }

    double correct = common::safe_at(_predictions, std::make_pair(label, label));
    
    if(rec != 0.0)
        rec = correct / rec;

    if(prec != 0.0)
        prec = correct / prec;

    if(prec + rec != 0.0)
        f1 = (2.0 * prec * rec) / (prec + rec);

    auto w = std::setw(_width);
    out << std::left << w << label
        << std::left << w << f1
        << std::left << w << prec
        << std::left << w << rec
        << std::endl;
}

void confusion_matrix::print_stats(std::ostream & out) const
{
    double t_prec = 0.0;
    double t_rec = 0.0;
    double t_f1 = 0.0;
    double t_corr = 0.0;

    auto w = std::setw(_width);
    out.precision(3);
    out << std::string(_width * 4, '-') << std::endl
        << std::left << w << "Class"
        << std::left << w << "F1 Score"
        << std::left << w << "Precision"
        << std::left << w << "Recall" << std::endl
        << std::string(_width * 4, '-') << std::endl;

    for(auto & cls: _classes)
    {
        double prec = 0.0, rec = 0.0, f1 = 0.0;
        auto it = _predictions.find(std::make_pair(cls, cls));
        if(it != _predictions.end())
            t_corr += it->second;
        print_class_stats(out, cls, prec, rec, f1);
        t_prec += prec;
        t_rec += rec;
        t_f1 += f1;
    }

    out << std::string(_width * 4, '-') << std::endl
        << w << "Total"
        << w << t_f1 / _classes.size()
        << w << t_prec / _classes.size()
        << w << t_rec / _classes.size() << std::endl
        << std::string(_width * 4, '-') << std::endl
        << _total << " predictions attempted, overall accuracy: "
        << t_corr / _total << std::endl;
}

double confusion_matrix::accuracy() const
{
    double correct = 0.0;
    for(auto & cls: _classes)
        correct += common::safe_at(_predictions, std::make_pair(cls, cls));
    return correct / _total;
}

size_t confusion_matrix::string_pair_hash(const std::pair<std::string, std::string> & str_pair)
{
    return std::hash<std::string>()(str_pair.first + str_pair.second);
}

const confusion_matrix::prediction_counts & confusion_matrix::predictions() const
{
    return _predictions;
}

confusion_matrix confusion_matrix::operator+(const confusion_matrix & other) const
{
    confusion_matrix sum{*this};
    
    for(auto & pred: other.predictions())
        sum.add(pred.first.first, pred.first.second, pred.second);

    return sum;
}

confusion_matrix & confusion_matrix::operator+=(const confusion_matrix & other)
{
    *this = *this + other;
    return *this;
}

bool confusion_matrix::mcnemar_significant(const confusion_matrix & a, const confusion_matrix & b)
{
    std::set<class_label> classes = a._classes;
    classes.insert(b._classes.begin(), b._classes.end());

    double a_adv = 0;
    double b_adv = 0;

    for(auto & cls: classes)
    {
        auto a_count = common::safe_at(a._predictions, std::make_pair(cls, cls));
        auto b_count = common::safe_at(b._predictions, std::make_pair(cls, cls));
        if(a_count > b_count)
            a_adv += (a_count - b_count);
        else if(b_count > a_count)
            b_adv += (b_count - a_count);
    }

    // does not approximate well if less than 25 samples
    if(a_adv + b_adv < 25)
        return false;

    double numerator = std::abs(a_adv - b_adv) - 0.5;
    double chi_square = (numerator * numerator) / (a_adv + b_adv);

    // check if significant with chi square, 1 degree of freedom, alpha == 0.05
    return chi_square > 3.84;
}

}
}
