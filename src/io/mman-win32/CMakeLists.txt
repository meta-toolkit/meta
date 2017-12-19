project(meta-mman-win32)

add_library(meta-mman-win32 mman.cpp)
target_link_libraries(meta-mman-win32 meta-definitions)

install(TARGETS meta-mman-win32
        EXPORT meta-exports
        DESTINATION lib)
