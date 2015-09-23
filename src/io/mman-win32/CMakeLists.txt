include_directories(${PROJECT_SOURCE_DIR}/../../include/io/mman-win32)
add_library(mman-win32 mman.c)
target_include_directories(mman-win32 PUBLIC
                           ${PROJECT_SOURCE_DIR}/../../include/io/mman-win32)
target_include_directories(meta-definitions INTERFACE
                           ${PROJECT_SOURCE_DIR}/../../include/io/mman-win32)
