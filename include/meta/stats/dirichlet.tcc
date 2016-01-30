/**
 * @file dirichlet.tcc
 * @author Chase Geigle
 */

#include "meta/stats/dirichlet.h"
#include "meta/util/identifiers.h"
#include "meta/util/shim.h"
#include "meta/io/packed.h"

namespace meta
{
namespace stats
{

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
    alpha_sum_
        = std::accumulate(begin, end, 0.0, [](double accum, const pair_type& b)
                          {
                              return accum + b.second;
                          });
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
    io::packed::write(out, static_cast<uint64_t>(type_));
    switch (type_)
    {
        case type::SYMMETRIC:
        {
            io::packed::write(out, params_.fixed_alpha_);
            io::packed::write(
                out, static_cast<uint64_t>(alpha_sum_ / params_.fixed_alpha_));
            break;
        }
        case type::ASYMMETRIC:
        {
            io::packed::write(out, params_.sparse_alpha_.size());
            for (const auto& alpha : params_.sparse_alpha_)
            {
                io::packed::write(out, alpha.first);
                io::packed::write(out, alpha.second);
            }
            break;
        }
    }
}

template <class T>
void dirichlet<T>::load(std::istream& in)
{
    uint64_t typ;
    auto bytes = io::packed::read(in, typ);
    if (bytes == 0)
        return;

    type read_type = static_cast<type>(typ);
    switch (read_type)
    {
        case type::SYMMETRIC:
        {
            double alpha;
            io::packed::read(in, alpha);
            uint64_t n;
            io::packed::read(in, n);
            *this = dirichlet{alpha, n};
            break;
        }
        case type::ASYMMETRIC:
        {
            uint64_t size;
            io::packed::read(in, size);
            std::vector<std::pair<T, double>> vec;
            vec.reserve(size);
            for (uint64_t i = 0; i < size; ++i)
            {
                T event;
                io::packed::read(in, event);
                double count;
                io::packed::read(in, count);
                vec.emplace_back(std::move(event), count);
            }
            *this = dirichlet{vec.begin(), vec.end()};
            break;
        }
    }
}
}
}
