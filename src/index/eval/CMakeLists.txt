project(meta-eval)

add_library(meta-eval ir_eval.cpp rank_correlation.cpp)
target_link_libraries(meta-eval meta-definitions cpptoml)

install(TARGETS meta-eval
        EXPORT meta-exports
        DESTINATION lib)
