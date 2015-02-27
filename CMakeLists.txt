cmake_minimum_required(VERSION 3.0.0)
project(meta)

option(USE_LIBCXX "Use libc++ for the C++ standard library" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(CTest)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)
include(ExternalProject)
include(FindZLIB)

include_directories(include/)


set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/deps/findicu)
find_package(Threads REQUIRED)
find_package(ICU COMPONENTS data i18n uc REQUIRED)
include_directories(SYSTEM ${ICU_INCLUDE_DIRS})

if(ZLIB_FOUND)
  include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
endif()

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

    # if we don't already set the standard for the compiler, detect the
    # best one available and use it
    if(NOT "${CMAKE_CXX_FLAGS}" MATCHES "std=c\\+\\+(0x|11|1y|14)")
        check_cxx_compiler_flag(-std=c++14 HAS_CXX14)
        if(HAS_CXX14)
            message("-- Compiler supports C++14 (using it)")
            set(STDOPT "-std=c++14")
        endif()

        if(NOT STDOPT)
            check_cxx_compiler_flag(-std=c++1y HAS_CXX1Y)
            if(HAS_CXX1Y)
                message("-- Compiler supports C++1y (using it)")
                set(STDOPT "-std=c++1y")
            endif()
        endif()

        if(NOT STDOPT)
            check_cxx_compiler_flag(-std=c++11 HAS_CXX11)
            if(HAS_CXX11)
                message("-- Compiler supports C++11 (using it)")
                set(STDOPT "-std=c++11")
            endif()
        endif()

        if(NOT STDOPT)
            check_cxx_compiler_flag(-std=c++0x HAS_CXX0X)
            if(HAS_CXXOX)
                message("-- Compiler supports C++0x (using it)")
                set(STDOPT "-std=c++0x")
            endif()
        endif()

        if(NOT STDOPT)
            message(FATAL_ERROR
                    "meta requires a compiler with at least C++0x support")
        endif()

        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STDOPT}")
    endif()

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        if(CMAKE_GENERATOR STREQUAL "Ninja")
          set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcolor-diagnostics")
        endif()
        if(USE_LIBCXX)
            message("-- Locating libc++...")
            find_library(LIBCXX_LIBRARY NAMES c++ cxx)
            if(LIBCXX_LIBRARY)
                message("-- Located libc++, using it.")
                set(LIBCXX_OPTIONS "-stdlib=libc++")
                message("--     Locating libc++'s abi...")
                find_library(LIBCXXABI_LIBRARY NAMES c++abi)
                find_library(LIBCXXRT_LIBRARY NAMES cxxrt)
                if(LIBCXXABI_LIBRARY)
                    message("--     Found libc++abi, using it.")
                    set(CXXABI_LIBRARY ${LIBCXXABI_LIBRARY})
                elseif(LIBCXXRT_LIBRARY)
                    message("--     Found libcxxrt, using it.")
                    set(CXXABI_LIBRARY ${LIBCXXRT_LIBRARY})
                else()
                    message("--     No abi library found. "
                            "Attempting to continue without one...")
                endif()
            else()
                message("-- Could not find libc++, will not use it.")
            endif()
        endif()
    endif()

    find_library(LIBDL_LIBRARY NAMES dl ldl)
    if(LIBDL_LIBRARY)
      set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${LIBDL_LIBRARY}")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIBCXX_OPTIONS}")
    set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${CXXABI_LIBRARY}")
endif()

add_library(meta-definitions INTERFACE)
target_include_directories(meta-definitions INTERFACE ${PROJECT_SOURCE_DIR}/include)

if(ZLIB_FOUND)
  target_include_directories(meta-definitions INTERFACE
                             ${ZLIB_INCLUDE_DIRS})
  target_compile_definitions(meta-definitions INTERFACE -DMETA_HAS_ZLIB)
endif()

if(HAS_CXX14 OR HAS_CXX1Y)
    target_compile_definitions(meta-definitions INTERFACE -DMETA_HAS_CXX14=1)
endif()

if(LIBDL_LIBRARY)
  target_link_libraries(meta-definitions INTERFACE ${LIBDL_LIBRARY})
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_compile_definitions(meta-definitions INTERFACE
                               -D_DARWIN_USE_64_BIT_INODE=1)
    target_compile_definitions(meta-definitions INTERFACE
                               -DMETA_IS_DARWIN=1)
endif()

check_cxx_source_compiles("
#include <atomic>
#include <memory>
int main () {
    auto sp = std::make_shared<int>(1);
    auto sp2 = std::atomic_load(&sp);
    return 0;
}" META_HAS_STD_SHARED_PTR_ATOMICS)

if(META_HAS_STD_SHARED_PTR_ATOMICS)
    target_compile_definitions(meta-definitions INTERFACE
                               -DMETA_HAS_STD_SHARED_PTR_ATOMICS=1)
endif()

check_cxx_source_compiles("
#include <fstream>
int main() {
    std::ofstream ofs{\"path\"};
    std::ofstream ofs2{std::move(ofs)};
    return 0;
}" META_HAS_STREAM_MOVE)

if(META_HAS_STREAM_MOVE)
    target_compile_definitions(meta-definitions INTERFACE
                               -DMETA_HAS_STREAM_MOVE=1)
endif()

check_cxx_source_compiles("
#include <memory>
int main() {
    auto i = std::make_unique<int>(1);
    return 0;
}" META_HAS_STD_MAKE_UNIQUE)

if(META_HAS_STD_MAKE_UNIQUE)
    target_compile_definitions(meta-definitions INTERFACE
                               -DMETA_HAS_STD_MAKE_UNIQUE)
endif()

if(ICU_VERSION VERSION_LESS "4.4")
  target_compile_definitions(meta-definitions INTERFACE
                             -DMETA_ICU_NO_TEMP_SUBSTRING)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

link_libraries(cpptoml meta-definitions)

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
