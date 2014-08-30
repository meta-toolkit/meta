cmake_minimum_required(VERSION 3.0.0)
project(meta)

option(USE_LIBCXX "Use libc++ for the C++ standard library" ON)

include_directories(include/)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/deps/findicu)
find_package(Threads REQUIRED)
find_package(ICU COMPONENTS data i18n uc REQUIRED)
include_directories(SYSTEM ${ICU_INCLUDE_DIRS})

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ldl")
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -ldl")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        if(USE_LIBCXX)
            message("-- Locating libc++...")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            find_library(META_LIBCXX NAMES c++ cxx)
            if(META_LIBCXX)
                message("-- Located libc++, using it.")
                set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++")
                set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -lc++")

                message("--     Locating libc++'s abi...")
                find_library(META_LIBCXXABI NAMES c++abi)
                find_library(META_LIBCXXRT NAMES cxxrt)
                if(META_LIBCXXABI)
                    message("--     Found libc++abi, using it.")
                    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++abi")
                    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -lc++abi")
                elseif(META_LIBCXXRT)
                    message("--     Found libcxxrt, using it.")
                    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lcxxrt")
                    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -lcxxrt")
                else()
                    message("--     No abi library found. "
                            "Attempting to continue without one...")
                endif()
            else()
                message("-- Could not find libc++, will not use it.")
            endif()
        endif()
    endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    add_definitions(-D_DARWIN_USE_64_BIT_INODE=1)
    add_definitions(-DMETA_IS_DARWIN=1)
endif()

include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)

check_cxx_source_compiles("
#include <atomic>
#include <memory>
int main () {
    auto sp = std::make_shared<int>(1);
    auto sp2 = std::atomic_load(&sp);
    return 0;
}" META_HAS_STD_SHARED_PTR_ATOMICS)

if(META_HAS_STD_SHARED_PTR_ATOMICS)
    add_definitions(-DMETA_HAS_STD_SHARED_PTR_ATOMICS=1)
endif()

check_cxx_source_compiles("
#include <fstream>
int main() {
    std::ofstream ofs{\"path\"};
    std::ofstream ofs2{std::move(ofs)};
    return 0;
}" META_HAS_STREAM_MOVE)

if(META_HAS_STREAM_MOVE)
    add_definitions(-DMETA_HAS_STREAM_MOVE=1)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

link_libraries(cpptoml)

add_custom_target(tidy
                  COMMAND /usr/bin/rm -rf
                  ./doc
                  *.terms
                  *.phi
                  *.theta)

find_package(Doxygen)
if(DOXYGEN_FOUND AND NOT TARGET doc)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/meta.doxygen.in
                   ${CMAKE_CURRENT_BINARY_DIR}/meta.doxygen @ONLY)
    add_custom_target(doc
                      ${DOXYGEN_EXECUTABLE}
                      ${CMAKE_CURRENT_BINARY_DIR}/meta.doxygen
                      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()

add_subdirectory(src)
add_subdirectory(deps/porter2_stemmer EXCLUDE_FROM_ALL)
add_subdirectory(deps/cpptoml EXCLUDE_FROM_ALL)
