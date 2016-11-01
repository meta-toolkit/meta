set -v
mkdir $HOME/lib
export LD_LIBRARY_PATH=$HOME/lib:$LD_LIBRARY_PATH
mkdir $HOME/bin
export PATH=$HOME/bin:$PATH
mkdir $HOME/include
export CPLUS_INCLUDE_PATH=$HOME/include:$CPLUS_INCLUDE_PATH

CMAKE_VERSION="${CMAKE_VERSION:-3.2.3}"
CMAKE_VERSION_PARTS=( ${CMAKE_VERSION//./ } )
CMAKE_MAJOR_MINOR="${CMAKE_VERSION_PARTS[0]}.${CMAKE_VERSION_PARTS[1]}"

wget --no-check-certificate https://www.cmake.org/files/v$CMAKE_MAJOR_MINOR/cmake-$CMAKE_VERSION-Linux-x86_64.sh
sh cmake-$CMAKE_VERSION-Linux-x86_64.sh --prefix=$HOME --exclude-subdir

# we have to manually set CC and CXX since travis 'helpfully' clobbers them
if [ "$COMPILER" = "gcc" ]; then
      export CC=gcc-$GCC_VERSION
      export CXX=g++-$GCC_VERSION
fi

if [ "$COMPILER" == "clang" ]; then
      export CC=clang-$CLANG_VERSION
      export CXX=clang++-$CLANG_VERSION
      travis/install_libcxx.sh
fi
set +v
