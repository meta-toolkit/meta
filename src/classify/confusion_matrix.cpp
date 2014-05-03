/**
 * @file confusion_matrix.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <iomanip>
#include <vector>

#include "classify/confusion_matrix.h"
#include "util/mapping.h"
#include "util/printing.h"

namespace meta
{
namespace classify
{

confusion_matrix::confusion_matrix()
    : predictions_{32, string_pair_hash}, total_{0}
{
    /* nothing */
}

void confusion_matrix::add(const class_label& predicted,
                           const class_label& actual, size_t times)
{
    std::pair<class_label, class_label> prediction{predicted, actual};
    predictions_[prediction] += times;
    counts_[actual] += times;
    classes_.insert(predicted);
    classes_.insert(actual);
    total_ += times;
}

void confusion_matrix::print_result_pairs(std::ostream& out) const
{
    for (auto& p : predictions_)
        for (size_t i = 0; i < p.second; ++i)
            out << p.first.first << " " << p.first.second << "\n";
}

void confusion_matrix::print(std::ostream& out) const
{
    auto saved_precision = out.precision();

    auto max_label = std::max_element(classes_.begin(), classes_.end(),
        [](const class_label& a, const class_label& b)
        {
            return static_cast<std::string>(a).size() <
                static_cast<std::string>(b).size();
        });

    size_t w = std::max<size_t>(7, static_cast<std::string>(*max_label).size())
               + 2;
    out << std::left << std::endl << std::setw(w) << "";
    out << "  ";
    for (auto& aClass : classes_)
        out << std::setw(w) << aClass;
    out << std::endl;
    out << std::string(w, ' ') << std::string(classes_.size() * w, '-')
        << std::endl;

    for (auto& aClass : classes_)
    {
        out << std::setw(w - 1) << std::right << aClass << std::left << " | ";
        for (auto& pClass : classes_)
        {
            auto predIt = predictions_.find(std::make_pair(pClass, aClass));
            if (predIt != predictions_.end())
            {
                size_t numPred = predIt->second;
                double percent = static_cast<double>(numPred)
                                 / counts_.at(aClass);
                out.precision(3);
                std::stringstream ss;
                ss.precision(3);
                ss << percent;
                if (aClass == pClass)
                {
                    auto str = printing::make_bold(ss.str());
                    auto diff = str.size() - ss.str().size();
                    out << std::setw(w + diff) << str;
                }
                else
                {
                    out << std::setw(w) << ss.str();
                }
            }
            else
                out << std::setw(w) << "- ";
        }
        out << std::endl;
    }
    out << std::endl;

    out.precision(saved_precision);
}

void confusion_matrix::print_class_stats(std::ostream& out,
                                         const class_label& label, double& prec,
                                         double& rec, double& f1,
                                         size_t width) const
{
    for (auto& cls : classes_)
    {
        prec += map::safe_at(predictions_, std::make_pair(cls, label));
        rec += map::safe_at(predictions_, std::make_pair(label, cls));
    }

    double correct = map::safe_at(predictions_, std::make_pair(label, label));

    if (rec != 0.0)
        rec = correct / rec;

    if (prec != 0.0)
        prec = correct / prec;

    if (prec + rec != 0.0)
        f1 = (2.0 * prec * rec) / (prec + rec);

    auto w1 = std::setw(width);
    auto w2 = std::setw(12);
    out << std::left << w1 << label << std::left << w2 << f1 << std::left << w2
        << prec << std::left << w2 << rec << std::endl;
}

void confusion_matrix::print_stats(std::ostream& out) const
{
    double t_prec = 0.0;
    double t_rec = 0.0;
    double t_f1 = 0.0;
    double t_corr = 0.0;
    auto saved_precision = out.precision();

    auto max_label
        = std::max_element(classes_.begin(), classes_.end(),
            [](const class_label& a, const class_label& b)
            {
                return static_cast<std::string>(a).size()
                    < static_cast<std::string>(b).size();
            });

    size_t width = std::max<size_t>(
            12, static_cast<std::string>(*max_label).size() + 2);

    auto w1 = std::setw(width + printing::make_bold("").size());
    auto w2 = std::setw(12 + printing::make_bold("").size());
    out.precision(3);
    out << std::string(width + 12 * 3, '-') << std::endl << std::left << w1
        << printing::make_bold("Class") << std::left << w2
        << printing::make_bold("F1 Score") << std::left << w2
        << printing::make_bold("Precision") << std::left << w2
        << printing::make_bold("Recall") << std::endl
        << std::string(width + 12 * 3, '-') << std::endl;

    for (auto& cls : classes_)
    {
        double prec = 0.0, rec = 0.0, f1 = 0.0;
        auto it = predictions_.find(std::make_pair(cls, cls));
        if (it != predictions_.end())
            t_corr += it->second;
        print_class_stats(out, cls, prec, rec, f1, width);
        t_prec += prec;
        t_rec += rec;
        t_f1 += f1;
    }

    auto limit = [](double val)
    {
        std::stringstream ss;
        ss.precision(3);
        ss << val;
        return ss.str();
    };

    out << std::string(width + 12 * 3, '-') << std::endl << w1
        << printing::make_bold("Total") << w2
        << printing::make_bold(limit(t_f1 / classes_.size())) << w2
        << printing::make_bold(limit(t_prec / classes_.size())) << w2
        << printing::make_bold(limit(t_rec / classes_.size())) << std::endl
        << std::string(width + 12 * 3, '-') << std::endl << total_
        << " predictions attempted, overall accuracy: " << t_corr / total_
        << std::endl;

    out.precision(saved_precision);
}

double confusion_matrix::accuracy() const
{
    double correct = 0.0;
    for (auto& cls : classes_)
        correct += map::safe_at(predictions_, std::make_pair(cls, cls));
    return correct / total_;
}

size_t confusion_matrix::string_pair_hash(const std::pair
                                          <std::string, std::string>& str_pair)
{
    return std::hash<std::string>()(str_pair.first + str_pair.second);
}

const confusion_matrix::prediction_counts& confusion_matrix::predictions() const
{
    return predictions_;
}

confusion_matrix confusion_matrix::operator+(const confusion_matrix
                                             & other) const
{
    confusion_matrix sum{*this};

    for (auto& pred : other.predictions())
        sum.add(pred.first.first, pred.first.second, pred.second);

    return sum;
}

confusion_matrix& confusion_matrix::operator+=(const confusion_matrix& other)
{
    *this = *this + other;
    return *this;
}

bool confusion_matrix::mcnemar_significant(const confusion_matrix& a,
                                           const confusion_matrix& b)
{
    std::set<class_label> classes = a.classes_;
    classes.insert(b.classes_.begin(), b.classes_.end());

    double a_adv = 0;
    double b_adv = 0;

    for (auto& cls : classes)
    {
        auto a_count = map::safe_at(a.predictions_, std::make_pair(cls, cls));
        auto b_count = map::safe_at(b.predictions_, std::make_pair(cls, cls));
        if (a_count > b_count)
            a_adv += (a_count - b_count);
        else if (b_count > a_count)
            b_adv += (b_count - a_count);
    }

    // does not approximate well if less than 25 samples
    if (a_adv + b_adv < 25)
        return false;

    double numerator = std::abs(a_adv - b_adv) - 0.5;
    double chi_square = (numerator * numerator) / (a_adv + b_adv);

    // check if significant with chi square, 1 degree of freedom, alpha == 0.05
    return chi_square > 3.84;
}
}
}
