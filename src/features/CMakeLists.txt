project(meta-features)

add_subdirectory(tools)

add_library(meta-features feature_selector.cpp
                          selector_factory.cpp
                          chi_square.cpp
                          odds_ratio.cpp
                          correlation_coefficient.cpp
                          information_gain.cpp)
target_link_libraries(meta-features meta-index ${CMAKE_THREAD_LIBS_INIT})
