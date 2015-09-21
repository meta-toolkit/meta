project(meta-kernel)

add_library(meta-kernel kernel_factory.cpp
                        polynomial.cpp
                        radial_basis.cpp
                        sigmoid.cpp)
