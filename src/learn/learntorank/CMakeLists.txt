PROJECT(meta-learntorank)

add_library(meta-learntorank letor.cpp)

set_target_properties(meta-learntorank PROPERTIES LINKER_LANGUAGE CXX)
#target_link_libraries(meta-learntorank meta-definitions)

install(TARGETS meta-learntorank
        EXPORT meta-exports
        DESTINATION lib)