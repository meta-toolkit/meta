cmake_minimum_required(VERSION 3.2.0)
project(meta)

option(USE_LIBCXX "Use libc++ for the C++ standard library" ON)
option(ENABLE_PROFILING "Link against gperftools profiler library" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(CTest)
include(CheckCXXCompilerFlag)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)
include(ExternalProject)
include(FindZLIB)

# Check if there is no build type set. If meta itself is the root project,
# compile it in release mode instead. If we aren't the root project, just
# continue along with whatever we would do ordinarily (they *really* should
# be specifying a build type, but...)
if (NOT CMAKE_BUILD_TYPE AND CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  message("-- No build type selected, defaulting to Release")
  set(CMAKE_BUILD_TYPE "Release")
endif()

include_directories(include/)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/deps/findicu)
find_package(Threads REQUIRED)
find_package(ICU 56.1 COMPONENTS data i18n uc)

if (NOT ICU_FOUND OR ICU_VERSION VERSION_LESS 56.1)
  message("-- ICU version ${ICU_VERSION} is too old; attempting to build ICU...")
  if (WIN32)
    # not going to attempt to build ICU if we're on Windows for now
    # probably could, but it's more trouble than it's worth I think
    message("-- ICU building not supported on Windows.")
    if (MINGW)
      message(FATAL_ERROR "   -- If on MSYS2; please install the icu package via pacman")
    else()
      message(FATAL_ERROR "   -- Please download the latest ICU binaries from http://site.icu-project.org/download")
    endif()
  elseif(UNIX)

    # determine platform for runConfigureICU
    if (APPLE)
      set(ICU_PLATFORM "MacOSX")
    else()
      set(ICU_PLATFORM "Linux")
    endif()

    # determine a reasonable number of threads to build ICU with
    include(ProcessorCount)
    ProcessorCount(CORES)
    if (NOT CORES EQUAL 0)
      # limit the number of cores to 4 on travis
      if (CORES GREATER 4)
        if ($ENV{TRAVIS})
          set(CORES 4)
        endif()
      endif()
      set(ICU_MAKE_EXTRA_FLAGS "-j${CORES}")
    endif()

    set(ICU_EP_PREFIX ${PROJECT_SOURCE_DIR}/deps/icu)

    ExternalProject_Add(ExternalICU
      PREFIX ${ICU_EP_PREFIX}
      URL http://download.icu-project.org/files/icu4c/56.1/icu4c-56_1-src.tgz
      URL_HASH MD5=c4a2d71ff56aec5ebfab2a3f059be99d
      CONFIGURE_COMMAND ${ICU_EP_PREFIX}/src/ExternalICU/source/runConfigureICU ${ICU_PLATFORM}
      --disable-shared --enable-static --disable-dyload --disable-extras
      --disable-tests --disable-samples
      --prefix=<INSTALL_DIR>
      BUILD_COMMAND make ${ICU_MAKE_EXTRA_FLAGS}
      INSTALL_COMMAND make install
      BUILD_BYPRODUCTS ${ICU_EP_PREFIX}/lib/libicudata.a;${ICU_EP_PREFIX}/lib/libicui18n.a;${ICU_EP_PREFIX}/lib/libicuuc.a
    )
    set(ICU_INCLUDE_DIRS ${ICU_EP_PREFIX}/include)

    add_library(icudata IMPORTED STATIC)
    set_target_properties(icudata PROPERTIES IMPORTED_LOCATION
      ${ICU_EP_PREFIX}/lib/libicudata.a)
    add_dependencies(icudata ExternalICU)

    add_library(icui18n IMPORTED STATIC)
    set_target_properties(icui18n PROPERTIES IMPORTED_LOCATION
      ${ICU_EP_PREFIX}/lib/libicui18n.a)
    add_dependencies(icui18n ExternalICU)

    add_library(icuuc IMPORTED STATIC)
    set_target_properties(icuuc PROPERTIES IMPORTED_LOCATION
      ${ICU_EP_PREFIX}/lib/libicuuc.a)
    add_dependencies(icuuc ExternalICU)

    set(ICU_LIBRARIES icui18n icuuc icudata)
    set(ICU_IS_EXTERNAL TRUE)
  else()
    message(FATAL_ERROR "-- ICU building not supported for this platform")
  endif()
endif()

message("-- ICU include dirs: ${ICU_INCLUDE_DIRS}")
message("-- ICU libraries: ${ICU_LIBRARIES}")

add_library(icu INTERFACE)
if (ICU_IS_EXTERNAL)
  file(MAKE_DIRECTORY ${ICU_INCLUDE_DIRS})
endif()
target_link_libraries(icu INTERFACE ${ICU_LIBRARIES})
target_include_directories(icu SYSTEM INTERFACE ${ICU_INCLUDE_DIRS})

#include_directories(SYSTEM ${ICU_INCLUDE_DIRS})

if(ZLIB_FOUND)
  include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
endif()

if(UNIX OR MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

    # Enable -Wconversion on clang, since it's not *too* noisy there.
    #
    # As of GCC 5.2.0, there are too many spurious warnings to bother
    # enabling this there.
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
    endif()

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
    endif()

    find_library(LIBDL_LIBRARY NAMES dl ldl)
    if(LIBDL_LIBRARY)
      set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${LIBDL_LIBRARY}")
    endif()

    if(LIBCXX_OPTIONS)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIBCXX_OPTIONS}")
    endif()

    if(CXXABI_LIBRARY)
      set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} ${LIBCXX_OPTIONS} ${CXXABI_LIBRARY} -L${LIBCXX_LIB_PATH}")
    endif()
    if(LIBCXX_INCLUDE_DIR)
      set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES} ${LIBCXX_INCLUDE_DIR}")
    endif()
endif()

add_library(meta-definitions INTERFACE)
target_include_directories(meta-definitions INTERFACE ${PROJECT_SOURCE_DIR}/include)

if (STDOPT)
  target_compile_options(meta-definitions INTERFACE ${STDOPT})
endif()

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
if(JEMALLOC_LIB)
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
    std::experimental::string_view sv = \"hello world\";
    return 0;
}" META_HAS_EXPERIMENTAL_STRING_VIEW)

if (META_HAS_EXPERIMENTAL_STRING_VIEW)
  target_compile_definitions(meta-definitions INTERFACE
                             -DMETA_HAS_EXPERIMENTAL_STRING_VIEW)
endif()

# work around a bug in libstdc++ provided with gcc < 4.8.3 where a static
# assertion fires when you have a non-empty hash functor
check_cxx_source_compiles("
#include <unordered_set>
struct nonempty_hasher : public std::hash<int> { int i = 3; };
int main() {
    std::unordered_set<int, nonempty_hasher> s;
    return 0;
}" META_HAS_NONEMPTY_HASH_SUPPORT)

if (META_HAS_NONEMPTY_HASH_SUPPORT)
  target_compile_definitions(meta-definitions INTERFACE
                             -DMETA_HAS_NONEMPTY_HASH_SUPPORT)
endif()

# work around a bug in gcc 4.8.1 where having a lambda as a default
# argument to a member function template causes an internal compiler error
check_cxx_source_compiles("
struct h {
  template <class Function = void(*)()>
  void foo(Function&& fun = [](){}) {
    fun();
  }
};

int main() {
  h b;
  b.foo();
  return 0;
}" META_HAS_LAMBDA_AS_DEFAULT_ARGUMENT)

if (META_HAS_LAMBDA_AS_DEFAULT_ARGUMENT)
  target_compile_definitions(meta-definitions INTERFACE
    -DMETA_HAS_MEM_FN_TEMPLATE_LAMBDA_DEFAULT_ARGUMENT)
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

check_cxx_source_compiles("
#include <experimental/filesystem>

int main()
{
    std::tr2::sys::filesystem::path p1 = \"/usr\";
    return 0;
}" META_HAS_TR2_SYS_FILESYSTEM)

if (META_HAS_TR2_SYS_FILESYSTEM)
  target_compile_definitions(meta-definitions INTERFACE
    -DMETA_HAS_TR2_SYS_FILESYSTEM)
endif()

if(NOT META_HAS_EXPERIMENTAL_FILESYSTEM AND NOT META_HAS_TR2_SYS_FILESYSTEM)
  target_include_directories(meta-definitions SYSTEM INTERFACE ${PROJECT_SOURCE_DIR}/deps/meta-stlsoft/include)
endif()

if(ICU_VERSION VERSION_LESS "4.4")
  target_compile_definitions(meta-definitions INTERFACE
                             -DMETA_ICU_NO_TEMP_SUBSTRING)
endif()

if(BIICODE)
  include(contrib/biicode/CMakeLists.txt)
  return()
ENDIF()

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
