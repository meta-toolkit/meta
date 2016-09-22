cmake_minimum_required(VERSION 3.2.0)
project(meta)

option(ENABLE_LIBCXX "Use libc++ for the C++ standard library (only for clang)" ON)
option(ENABLE_PROFILING "Link against gperftools profiler library" OFF)
option(ENABLE_JEMALLOC "Link against jemalloc if available" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(MeTA_VERSION_MAJOR 2)
set(MeTA_VERSION_MINOR 4)
set(MeTA_VERSION_PATCH 2)
set(MeTA_VERSION
    "${MeTA_VERSION_MAJOR}.${MeTA_VERSION_MINOR}.${MeTA_VERSION_PATCH}")

# MeTA only requires C++11, but on all of our supported compilers at least
# C++1y support exists, so we therefore set the standard to 14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(META_PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR})

include(CMakePushCheckState)
include(ExternalProject)
include(deps/meta-cmake/FindOrBuildICU.cmake)
include(deps/meta-cmake/SetClangOptions.cmake)
include(deps/meta-cmake/CompilerKludges.cmake)

find_package(Threads REQUIRED)
find_package(ZLIB REQUIRED)

cmake_push_check_state()

# Work around CMake not propagating the standard flag down to the compiler
set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${CMAKE_CXX14_STANDARD_COMPILE_OPTION}")

# Check if there is no build type set. If meta itself is the root project,
# compile it in release mode instead. If we aren't the root project, just
# continue along with whatever we would do ordinarily (they *really* should
# be specifying a build type, but...)
if (NOT CMAKE_BUILD_TYPE AND CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  message("-- No build type selected, defaulting to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/findicu)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/deps/meta-cmake/)

# We require Unicode 8 for the unit tests, which was added in ICU 56.1
FindOrBuildICU(
  VERSION 57.1
  URL http://download.icu-project.org/files/icu4c/57.1/icu4c-57_1-src.tgz
  URL_HASH MD5=976734806026a4ef8bdd17937c8898b9
)

add_library(meta-definitions INTERFACE)
target_include_directories(meta-definitions INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

if(UNIX OR MINGW)
  target_compile_options(meta-definitions INTERFACE -Wall -Wextra -pedantic)

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    SetClangOptions(meta-definitions)
  endif()
endif()

target_include_directories(meta-definitions SYSTEM INTERFACE ${ZLIB_INCLUDE_DIRS})

if (LIBDL_LIBRARY)
  target_link_libraries(meta-definitions INTERFACE ${LIBDL_LIBRARY})
endif()

if (CXXABI_LIBRARY)
  target_link_libraries(meta-definitions INTERFACE ${CXXABI_LIBRARY})
endif()

if (LIBCXX_FOUND)
  target_include_directories(meta-definitions SYSTEM INTERFACE ${LIBCXX_INCLUDE_DIR})
  target_compile_options(meta-definitions INTERFACE ${LIBCXX_OPTIONS})
  target_link_libraries(meta-definitions INTERFACE -L${LIBCXX_LIB_PATH})
  target_link_libraries(meta-definitions INTERFACE ${LIBCXX_LIBRARY})
endif()

if (ENABLE_PROFILING)
  find_library(GPERFTOOLS_PROFILER NAMES profiler REQUIRED)
  message("-- Found profiler: ${GPERFTOOLS_PROFILER}")
  target_link_libraries(meta-definitions INTERFACE ${GPERFTOOLS_PROFILER})
endif()

find_library(JEMALLOC_LIB NAMES jemalloc)
if (JEMALLOC_LIB AND ENABLE_JEMALLOC)
  message("-- Using jemalloc: ${JEMALLOC_LIB}")
  target_link_libraries(meta-definitions INTERFACE ${JEMALLOC_LIB})
else()
  message("-- Using regular malloc; consider installing jemalloc")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(_DARWIN_USE_64_BIT_INODE 1)
  set(META_IS_DARWIN 1)
  target_compile_definitions(meta-definitions INTERFACE
                             -D_DARWIN_USE_64_BIT_INODE=1)
  target_compile_definitions(meta-definitions INTERFACE
                             -DMETA_IS_DARWIN=1)
endif()

# set a bunch of preprocessor variables to work around various compiler and
# standard library bugs
CompilerKludges(meta/kludges.h)

configure_file(include/meta/config.h.in meta/config.h)
target_include_directories(meta-definitions INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>)

cmake_pop_check_state()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

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
add_subdirectory(tests)
add_subdirectory(deps/cpptoml EXCLUDE_FROM_ALL)

# install our targets defined in this file
install(TARGETS meta-definitions
        EXPORT meta-exports
        DESTINATION lib)

# install all of our includes
install(DIRECTORY include/meta/
        DESTINATION include/meta
        FILES_MATCHING PATTERN "*.h"
                       PATTERN "*.tcc")
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/meta/
        DESTINATION include/meta
        FILES_MATCHING PATTERN "*.h")

# generate our ConfigVersion.cmake file
configure_file(cmake/MeTAConfigVersion.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/MeTA/MeTAConfigVersion.cmake
               @ONLY)
configure_file(cmake/MeTAConfig.cmake.in
               ${CMAKE_CURRENT_BINARY_DIR}/MeTA/MeTAConfig.cmake
               COPYONLY)

# install our exports and cmake files
install(EXPORT meta-exports
        FILE MeTATargets.cmake
        DESTINATION lib/cmake/MeTA)
install(FILES
          ${CMAKE_CURRENT_BINARY_DIR}/MeTA/MeTAConfigVersion.cmake
          ${CMAKE_CURRENT_BINARY_DIR}/MeTA/MeTAConfig.cmake
        DESTINATION
          lib/cmake/MeTA)

# allow consumption of a build directory as an "installed" version
export(EXPORT meta-exports
       FILE ${CMAKE_CURRENT_BINARY_DIR}/MeTA/MeTATargets.cmake)
export(PACKAGE MeTA)
