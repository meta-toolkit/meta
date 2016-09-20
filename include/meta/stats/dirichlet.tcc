/**
 * @file dirichlet.tcc
 * @author Chase Geigle
 */

#include "meta/io/packed.h"
#include "meta/stats/dirichlet.h"
#include "meta/util/identifiers.h"
#include "meta/util/shim.h"

namespace meta
{
namespace stats
{

template <class T>
dirichlet<T>::dirichlet() : dirichlet{0.0, 0}
{
    // nothing
}

template <class T>
dirichlet<T>::dirichlet(double alpha, uint64_t n)
    : type_{type::SYMMETRIC}, params_{alpha}, alpha_sum_{n * alpha}
{
    // nothing
}

template <class T>
template <class Iter>
dirichlet<T>::dirichlet(Iter begin, Iter end)
    : type_{type::ASYMMETRIC}, params_{begin, end}
{
    using pair_type = typename Iter::value_type;
    alpha_sum_ = std::accumulate(
        begin, end, 0.0,
        [](double accum, const pair_type& b) { return accum + b.second; });
}

template <class T>
dirichlet<T>::dirichlet(const dirichlet& other)
    : type_{other.type_}, alpha_sum_{other.alpha_sum_}
{
    switch (type_)
    {
        case type::SYMMETRIC:
            params_.fixed_alpha_ = other.params_.fixed_alpha_;
            return;
        case type::ASYMMETRIC:
            new (&params_.sparse_alpha_)
                util::sparse_vector<T, double>{other.params_.sparse_alpha_};
            return;
    }
}

template <class T>
dirichlet<T>::dirichlet(dirichlet&& other)
    : type_{other.type_}, alpha_sum_{other.alpha_sum_}
{
    switch (type_)
    {
        case type::SYMMETRIC:
            params_.fixed_alpha_ = other.params_.fixed_alpha_;
            return;
        case type::ASYMMETRIC:
            new (&params_.sparse_alpha_) util::sparse_vector<T, double>{
                std::move(other.params_.sparse_alpha_)};
            return;
    }
}

template <class T>
dirichlet<T>& dirichlet<T>::operator=(dirichlet rhs)
{
    swap(rhs);
    return *this;
}

template <class T>
dirichlet<T>::~dirichlet()
{
    switch (type_)
    {
        case type::SYMMETRIC:
            return;
        case type::ASYMMETRIC:
            params_.sparse_alpha_.~sparse_vector();
            return;
    }
}

template <class T>
double dirichlet<T>::pseudo_counts(const T& event) const
{
    switch (type_)
    {
        case type::SYMMETRIC:
            return params_.fixed_alpha_;
        case type::ASYMMETRIC:
            return params_.sparse_alpha_.at(event);
    }
    return 0.0; // unreachable
}

template <class T>
double dirichlet<T>::pseudo_counts() const
{
    return alpha_sum_;
}

template <class T>
void dirichlet<T>::swap(dirichlet& other)
{
    if (type_ != other.type_)
    {
        switch (type_)
        {
            case type::SYMMETRIC:
                // other is ASYMMETRIC
                new (&params_.sparse_alpha_) util::sparse_vector<T, double>{
                    std::move(other.params_.sparse_alpha_)};
                other.params_.sparse_alpha_.~sparse_vector();
                break;
            case type::ASYMMETRIC:
                // other is SYMMETRIC
                new (&other.params_.sparse_alpha_)
                    util::sparse_vector<T, double>{
                        std::move(params_.sparse_alpha_)};
                params_.sparse_alpha_.~sparse_vector();
                break;
        }
        std::swap(type_, other.type_);
    }
    else
    {
        switch (type_)
        {
            case type::SYMMETRIC:
                std::swap(params_.fixed_alpha_, other.params_.fixed_alpha_);
                break;
            case type::ASYMMETRIC:
                std::swap(params_.sparse_alpha_, other.params_.sparse_alpha_);
                break;
        }
    }
    std::swap(alpha_sum_, other.alpha_sum_);
}

template <class T>
void dirichlet<T>::save(std::ostream& out) const
{
    io::packed::write(out, *this);
}

template <class T>
void dirichlet<T>::load(std::istream& in)
{
    io::packed::read(in, *this);
}
}
}
