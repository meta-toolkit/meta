/* 
 * test case from
 * http://stackoverflow.com/questions/14044539/a-parallel-for-using-stdthread
 */

#include <iomanip>
#include <iostream>
#include <cmath>
#include <numeric>

#include "parallel/parallel_for.h"

using namespace parallel;

template <class Type>
void f( Type & x ) {
    x = std::sin(x)+std::exp(std::cos(x))/std::exp(std::sin(x)); 
}

int main() {
    double x = 0;
    double y = 0;
    size_t n = 100000000;
    std::vector<double> v(n);

    auto time = std::chrono::system_clock::now();
    std::iota(v.begin(), v.end(), 0);
    std::for_each(v.begin(), v.end(), f<double>);
    for (unsigned int i = 0; i < n; ++i) x += v[i];
    auto fin = std::chrono::system_clock::now();
    std::cout << "Serial version took: " 
        << std::chrono::duration_cast<std::chrono::seconds>( fin - time ).count() 
        << " seconds..." << std::endl;

    time = std::chrono::system_clock::now();
    std::iota(v.begin(), v.end(), 0);
    parallel_for(v.begin(), v.end(), f<double>);
    for (unsigned int i = 0; i < n; ++i) y += v[i];
    fin = std::chrono::system_clock::now();
    std::cout << "Parallel version took: " 
        << std::chrono::duration_cast<std::chrono::seconds>( fin - time ).count() 
        << " seconds..." << std::endl;

    std::cout << std::setprecision(15) << x << " " << y<< std::endl;
    return 0;
}
