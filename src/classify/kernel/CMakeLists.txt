project(meta-kernel)

add_library(meta-kernel kernel_factory.cpp
                        polynomial.cpp
                        radial_basis.cpp
                        sigmoid.cpp)
target_link_libraries(meta-kernel meta-definitions cpptoml)

install(TARGETS meta-kernel
        EXPORT meta-exports
        DESTINATION lib)
