project(meta-regression)

add_subdirectory(tools)

add_library(meta-regression metrics.cpp
                            regressor_factory.cpp
                            models/regressor.cpp
                            models/sgd.cpp)
target_link_libraries(meta-regression meta-learn)

install(TARGETS meta-regression
        EXPORT meta-exports
        DESTINATION lib)
