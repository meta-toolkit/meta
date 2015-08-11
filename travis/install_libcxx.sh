#!/bin/bash
set -x
cwd=$(pwd)
svn co --quiet http://llvm.org/svn/llvm-project/libcxx/trunk libcxx
git clone https://github.com/pathscale/libcxxrt.git libcxxrt
cd libcxxrt
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
cp lib/libcxxrt.so $HOME/lib
ln -sf $HOME/lib/libcxxrt.so $HOME/lib/libcxxrt.so.1
ln -sf $HOME/lib/libcxxrt.so $HOME/lib/libcxxrt.so.1.0
cd $cwd
cd libcxx
cd cmake/Modules
# HORRIBLE TERRIBLE NO GOOD VERY BAD
# hack the HandleOutOfTreeLLVM.cmake module file to allow us to actually
# specify a cmake path
patch -u HandleOutOfTreeLLVM.cmake $cwd/travis/HandleOutOfTreeLLVM.patch
cd ../../
mkdir build
cd build
cmake -DLIBCXX_CXX_ABI=libcxxrt \
      -DLIBCXX_CXX_ABI_INCLUDE_PATHS="../../libcxxrt/src" \
      -DLIBCXX_CXX_ABI_LIBRARY_PATH=$HOME/lib \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=$HOME \
      -DLLVM_CONFIG=/usr/bin/llvm-config-3.6 \
      -DLLVM_CMAKE_PATH=/usr/share/llvm-3.6/cmake \
      ..
make
make install
cd $cwd
