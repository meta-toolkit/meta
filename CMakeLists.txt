cmake_minimum_required(VERSION 3.2.0)
project(meta)

option(ENABLE_LIBCXX "Use libc++ for the C++ standard library (only for clang)" ON)
option(ENABLE_PROFILING "Link against gperftools profiler library" OFF)
option(ENABLE_JEMALLOC "Link against jemalloc if available" ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# MeTA only requires C++11, but on all of our supported compilers at least
# C++1y support exists, so we therefore set the standard to 14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)
include(CMakePushCheckState)
include(ExternalProject)
include(cmake/FindOrBuildICU.cmake)

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

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/deps/findicu)

# We require Unicode 8 for the unit tests, which was added in ICU 56.1
FindOrBuildICU(
  VERSION 56.1
  URL http://download.icu-project.org/files/icu4c/56.1/icu4c-56_1-src.tgz
  URL_HASH MD5=c4a2d71ff56aec5ebfab2a3f059be99d
)

add_library(meta-definitions INTERFACE)
target_include_directories(meta-definitions INTERFACE ${PROJECT_SOURCE_DIR}/include)

if(UNIX OR MINGW)
  target_compile_options(meta-definitions INTERFACE -Wall -Wextra -pedantic)

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Enable -Wconversion on clang, since it's not *too* noisy there.
    #
    # As of GCC 5.2.0, there are still too many spurious warnings to bother
    # enabling this there.
    target_compile_options(meta-definitions INTERFACE "-Wconversion")

    if(CMAKE_GENERATOR STREQUAL "Ninja")
      target_compile_options(meta-definitions INTERFACE "-fcolor-diagnostics")
    endif()

    if(ENABLE_LIBCXX)
      message("-- Locating libc++...")
      find_library(LIBCXX_LIBRARY NAMES c++ cxx)
      if(LIBCXX_LIBRARY)
        message("-- Located libc++: ${LIBCXX_LIBRARY}")
        set(LIBCXX_OPTIONS "-stdlib=libc++")
        get_filename_component(LIBCXX_LIB_PATH ${LIBCXX_LIBRARY}
                               DIRECTORY)
        find_path(LIBCXX_PREFIX c++/v1/algorithm
                  PATHS ${LIBCXX_LIB_PATH}/../include
                  ${CMAKE_SYSTEM_PREFIX_PATH})
        set(LIBCXX_INCLUDE_DIR ${LIBCXX_PREFIX}/c++/v1/)
        message("-- Located libc++ include path: ${LIBCXX_INCLUDE_DIR}")

        message("--     Locating libc++'s abi...")
        find_library(LIBCXXABI_LIBRARY NAMES c++abi)
        find_library(LIBCXXRT_LIBRARY NAMES cxxrt)
        if(LIBCXXABI_LIBRARY)
          message("--     Found libc++abi: ${LIBCXXABI_LIBRARY}")
          set(CXXABI_LIBRARY ${LIBCXXABI_LIBRARY})
        elseif(LIBCXXRT_LIBRARY)
          message("--     Found libcxxrt: ${LIBCXXRT_LIBRARY}")
          set(CXXABI_LIBRARY ${LIBCXXRT_LIBRARY})
        else()
          message("--     No abi library found. "
                  "Attempting to continue without one...")
        endif()
      else()
        message("-- Could not find libc++, will not use it.")
      endif()
    endif()

    find_library(LIBDL_LIBRARY NAMES dl ldl)
    if(LIBDL_LIBRARY)
      set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${LIBDL_LIBRARY}")
    endif()

    if(LIBCXX_OPTIONS)
      set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${LIBCXX_OPTIONS}")
    endif()

    if(CXXABI_LIBRARY)
      set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${LIBCXX_OPTIONS} ${CXXABI_LIBRARY} -L${LIBCXX_LIB_PATH}")
    endif()
    if(LIBCXX_INCLUDE_DIR)
      set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES} ${LIBCXX_INCLUDE_DIR}")
    endif()
  endif()
endif()

if(STDOPT)
  target_compile_options(meta-definitions INTERFACE ${STDOPT})
endif()

target_include_directories(meta-definitions SYSTEM INTERFACE ${ZLIB_INCLUDE_DIRS})

if(LIBDL_LIBRARY)
  target_link_libraries(meta-definitions INTERFACE ${LIBDL_LIBRARY})
endif()

if(CXXABI_LIBRARY)
  target_link_libraries(meta-definitions INTERFACE ${CXXABI_LIBRARY})
endif()

if(LIBCXX_LIBRARY)
  target_include_directories(meta-definitions SYSTEM INTERFACE ${LIBCXX_INCLUDE_DIR})
  target_compile_options(meta-definitions INTERFACE ${LIBCXX_OPTIONS})
  target_link_libraries(meta-definitions INTERFACE -L${LIBCXX_LIB_PATH})
  target_link_libraries(meta-definitions INTERFACE ${LIBCXX_LIBRARY})
endif()

if(ENABLE_PROFILING)
  find_library(GPERFTOOLS_PROFILER NAMES profiler REQUIRED)
  message("-- Found profiler: ${GPERFTOOLS_PROFILER}")
  target_link_libraries(meta-definitions INTERFACE ${GPERFTOOLS_PROFILER})
endif()

find_library(JEMALLOC_LIB NAMES jemalloc)
if(JEMALLOC_LIB AND ENABLE_JEMALLOC)
  message("-- Using jemalloc: ${JEMALLOC_LIB}")
  target_link_libraries(meta-definitions INTERFACE ${JEMALLOC_LIB})
else()
  message("-- Using regular malloc; consider installing jemalloc")
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

check_cxx_source_compiles("
#include <experimental/optional>
int main() {
    std::experimental::optional<int> x;
    return 0;
}" META_HAS_EXPERIMENTAL_OPTIONAL)

if (META_HAS_EXPERIMENTAL_OPTIONAL)
  target_compile_definitions(meta-definitions INTERFACE
                             -DMETA_HAS_EXPERIMENTAL_OPTIONAL)
endif()

check_cxx_source_compiles("
#include <experimental/string_view>
int main() {
    const std::experimental::string_view sv = \"hello world\";
    // test that string_view has to_string() const method
    // Xcode 6.4 appears to have shipped a string_view without it
    auto str = sv.to_string();
    return 0;
}" META_HAS_EXPERIMENTAL_STRING_VIEW)

if (META_HAS_EXPERIMENTAL_STRING_VIEW)
  target_compile_definitions(meta-definitions INTERFACE
                             -DMETA_HAS_EXPERIMENTAL_STRING_VIEW)
endif()

check_cxx_source_compiles("
#include <experimental/filesystem>

int main()
{
    std::experimental::filesystem::path p1 = \"/usr\";
    return 0;
}" META_HAS_EXPERIMENTAL_FILESYSTEM)

if (META_HAS_EXPERIMENTAL_FILESYSTEM)
  target_compile_definitions(meta-definitions INTERFACE
    -DMETA_HAS_EXPERIMENTAL_FILESYSTEM)
endif()

if(NOT META_HAS_EXPERIMENTAL_FILESYSTEM)
  target_include_directories(meta-definitions SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/deps/meta-stlsoft/include)
endif()

set(META_FOUND_ALIGNED_ALLOC_IMPL 0)

check_cxx_source_compiles("
#include <cstdlib>

int main()
{
    ::aligned_alloc(64, 128);
    return 0;
}" META_HAS_ALIGNED_ALLOC)

if (META_HAS_ALIGNED_ALLOC)
  set(META_FOUND_ALIGNED_ALLOC_IMPL 1)
  target_compile_definitions(meta-definitions INTERFACE
    -DMETA_HAS_ALIGNED_ALLOC)
endif()

if (NOT META_FOUND_ALIGNED_ALLOC_IMPL AND UNIX)
  check_cxx_source_compiles("
  #include <cstdlib>

  int main()
  {
      void* ptr;
      ::posix_memalign(&ptr, 64, 128);
      return 0;
  }" META_HAS_POSIX_MEMALIGN)

  if (META_HAS_POSIX_MEMALIGN)
    set(META_FOUND_ALIGNED_ALLOC_IMPL 1)
    target_compile_definitions(meta-definitions INTERFACE
      -DMETA_HAS_POSIX_MEMALIGN)
  endif()
endif()

if (NOT META_FOUND_ALIGNED_ALLOC_IMPL AND WIN32)
  check_cxx_source_compiles("
  #include <malloc.h>

  int main()
  {
      ::_aligned_malloc(128, 64);
      return 0;
  }" META_HAS_ALIGNED_MALLOC)

  if (META_HAS_ALIGNED_MALLOC)
    set(META_FOUND_ALIGNED_ALLOC_IMPL 1)
    target_compile_definitions(meta-definitions INTERFACE
      -DMETA_HAS_ALIGNED_MALLOC)
  endif()
endif()

if (NOT META_FOUND_ALIGNED_ALLOC_IMPL)
  message(FATAL_ERROR "Failed to find a suitable aligned allocation routine")
endif()

check_cxx_source_compiles("
int main()
{
    long x = 1;
    if (__builtin_expect(x == 1, 0))
        return 1;
    return 0;
}" META_HAS_BUILTIN_EXPECT)

if (META_HAS_BUILTIN_EXPECT)
  target_compile_definitions(meta-definitions INTERFACE
    -DMETA_HAS_BUILTIN_EXPECT)
endif()
cmake_pop_check_state()

if(BIICODE)
  include(contrib/biicode/CMakeLists.txt)
  return()
ENDIF()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

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
