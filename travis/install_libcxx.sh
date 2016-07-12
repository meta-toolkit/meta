#!/bin/bash
set -v
cwd=$(pwd)
svn co --quiet http://llvm.org/svn/llvm-project/llvm/trunk llvm
cd llvm/projects
svn co --quiet http://llvm.org/svn/llvm-project/libcxx/trunk libcxx
# Install libc++experimental.a alongside libc++.
sed -i '/LIBCXX_INSTALL_EXPERIMENTAL_LIBRARY/{s/OFF/ON/;}' libcxx/CMakeLists.txt
svn co --quiet http://llvm.org/svn/llvm-project/libcxxabi/trunk libcxxabi
cd ../
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME ../
make cxx
make install-libcxx install-libcxxabi
cd $cwd
set +v
