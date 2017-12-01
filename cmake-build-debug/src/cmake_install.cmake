# Install script for directory: /Users/mihikadave/Documents/CS510_MP/meta/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/analyzers/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/corpus/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/embeddings/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/features/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/graph/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/index/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/io/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/learn/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/lm/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/regression/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/sequence/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/stats/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/succinct/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/tools/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/topics/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/utf/cmake_install.cmake")
  include("/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/util/cmake_install.cmake")

endif()

