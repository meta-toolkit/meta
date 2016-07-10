/**
 * @file confusion_matrix.cpp
 * @author Sean Massung
 */

#include <algorithm>
#include <iomanip>
#include <vector>

#include "meta/classify/confusion_matrix.h"
#include "meta/util/mapping.h"
#include "meta/util/printing.h"

namespace meta
{
namespace classify
{

confusion_matrix::confusion_matrix()
    : predictions_{32, string_pair_hash}, total_{0}
{
    /* nothing */
}

void confusion_matrix::add(const predicted_label& predicted,
                           const class_label& actual, size_t times)
{
    std::pair<predicted_label, class_label> prediction{predicted, actual};
    predictions_[prediction] += times;
    counts_[actual] += times;
    classes_.insert(actual);
    total_ += times;
}

void confusion_matrix::print_result_pairs(std::ostream& out) const
{
    for (auto& p : predictions_)
        for (size_t i = 0; i < p.second; ++i)
            out << p.first.first << " " << p.first.second << "\n";
}

double confusion_matrix::f1_score() const
{
    auto p = precision();
    auto r = recall();
    if (p + r != 0.0)
        return (2.0 * p * r) / (p + r);

    return 0.0;
}

double confusion_matrix::f1_score(const class_label& lbl) const
{
    auto p = precision(lbl);
    auto r = recall(lbl);
    if (p + r != 0.0)
        return (2.0 * p * r) / (p + r);

    return 0.0;
}

double confusion_matrix::precision() const
{
    double sum = 0.0; // weighted sum of precision() scores by class
    for (auto& cls : classes_)
        sum += (static_cast<double>(counts_.at(cls)) / total_) * precision(cls);

    return sum;
}

double confusion_matrix::precision(const class_label& lbl) const
{
    double denom = 0.0;
    for (auto& cls : classes_)
        denom += map::safe_at(predictions_,
                              std::make_pair(predicted_label{lbl}, cls));

    double correct
        = map::safe_at(predictions_, std::make_pair(predicted_label{lbl}, lbl));

    if (denom != 0.0)
        return correct / denom;

    return 0.0;
}

double confusion_matrix::recall() const
{
    double sum = 0.0; // weighted sum of recall() scores by class
    for (auto& cls : classes_)
        sum += (static_cast<double>(counts_.at(cls)) / total_) * recall(cls);

    return sum;
}

double confusion_matrix::recall(const class_label& lbl) const
{
    double denom = 0.0;
    for (auto& cls : classes_)
        denom += map::safe_at(predictions_,
                              std::make_pair(predicted_label{cls}, lbl));

    double correct
        = map::safe_at(predictions_, std::make_pair(predicted_label{lbl}, lbl));

    if (denom != 0.0)
        return correct / denom;

    return 0.0;
}

void confusion_matrix::print(std::ostream& out) const
{
    auto saved_precision = out.precision();

    auto max_label
        = std::max_element(classes_.begin(), classes_.end(),
                           [](const class_label& a, const class_label& b)
                           {
                               return static_cast<std::string>(a).size()
                                      < static_cast<std::string>(b).size();
                           });

    auto w
        = std::max<size_t>(7, static_cast<std::string>(*max_label).size()) + 2;
    out << std::left << std::endl << std::setw(static_cast<int>(w)) << "";
    out << "  ";
    for (auto& actual_class : classes_)
        out << std::setw(static_cast<int>(w)) << actual_class;
    out << std::endl;
    out << std::string(w, ' ') << std::string(classes_.size() * w, '-')
        << std::endl;

    for (auto& actual_class : classes_)
    {
        out << std::setw(static_cast<int>(w - 1)) << std::right << actual_class
            << std::left << " | ";
        for (auto& pred_class : classes_)
        {
            auto it = predictions_.find(
                std::make_pair(predicted_label{pred_class}, actual_class));
            if (it != predictions_.end())
            {
                double percent = static_cast<double>(it->second)
                                 / counts_.at(actual_class);
                out.precision(3);
                std::stringstream ss;
                ss.precision(3);
                ss << percent;
                if (actual_class == pred_class)
                {
                    auto str = printing::make_bold(ss.str());
                    auto diff = str.size() - ss.str().size();
                    out << std::setw(static_cast<int>(w + diff)) << str;
                }
                else
                {
                    out << std::setw(static_cast<int>(w)) << ss.str();
                }
            }
            else
                out << std::setw(static_cast<int>(w)) << "- ";
        }
        out << std::endl;
    }
    out << std::endl;

    out.precision(saved_precision);
}

void confusion_matrix::print_stats(std::ostream& out) const
{
    auto saved_precision = out.precision();

    auto max_label
        = std::max_element(classes_.begin(), classes_.end(),
                           [](const class_label& a, const class_label& b)
                           {
                               return static_cast<std::string>(a).size()
                                      < static_cast<std::string>(b).size();
                           });

    size_t width
        = std::max<size_t>(12, static_cast<std::string>(*max_label).size() + 2);

    auto w1
        = std::setw(static_cast<int>(width + printing::make_bold("").size()));
    auto w2 = std::setw(static_cast<int>(12 + printing::make_bold("").size()));
    out.precision(3);
    out << std::string(width + 12 * 4, '-') << std::endl
        << std::left << w1 << printing::make_bold("Class") << std::left << w2
        << printing::make_bold("F1 Score") << std::left << w2
        << printing::make_bold("Precision") << std::left << w2
        << printing::make_bold("Recall") << std::left << w2
        << printing::make_bold("Class Dist") << std::endl
        << std::string(width + 12 * 4, '-') << std::endl;

    for (auto& cls : classes_)
    {
        auto w3 = std::setw(12); // different width for non-bold
        out << std::left << std::setw(static_cast<int>(width)) << cls
            << std::left << w3 << f1_score(cls) << std::left << w3
            << precision(cls) << std::left << w3 << recall(cls)
            << std::left << w3 << static_cast<double>(counts_.at(cls)) / total_
            << std::endl;
    }

    auto limit = [](double val)
    {
        std::stringstream ss;
        ss.precision(3);
        ss << val;
        return ss.str();
    };

    out << std::string(width + 12 * 4, '-') << std::endl
        << w1 << printing::make_bold("Total") << w2
        << printing::make_bold(limit(f1_score())) << w2
        << printing::make_bold(limit(precision())) << w2
        << printing::make_bold(limit(recall())) << std::endl
        << std::string(width + 12 * 4, '-') << std::endl
        << total_ << " predictions attempted, overall accuracy: " << accuracy()
        << std::endl;

    out.precision(saved_precision);
}

double confusion_matrix::accuracy() const
{
    double correct = 0.0;
    for (auto& cls : classes_)
        correct += map::safe_at(predictions_,
                                std::make_pair(predicted_label{cls}, cls));
    return correct / total_;
}

size_t confusion_matrix::string_pair_hash(
    const std::pair<std::string, std::string>& str_pair)
{
    return std::hash<std::string>()(str_pair.first + str_pair.second);
}

const confusion_matrix::prediction_counts& confusion_matrix::predictions() const
{
    return predictions_;
}

confusion_matrix confusion_matrix::
operator+(const confusion_matrix& other) const
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
        auto a_count = map::safe_at(a.predictions_,
                                    std::make_pair(predicted_label{cls}, cls));
        auto b_count = map::safe_at(b.predictions_,
                                    std::make_pair(predicted_label{cls}, cls));
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
