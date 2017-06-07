set -v
brew update
brew install icu4c jemalloc

# Travis Xcode 7.1+ images don't have cmake installed for some reason
if ! [ -x "$(command -v cmake)" ]; then
    brew install cmake
else
    brew outdated cmake || brew upgrade cmake
fi

if [ "$COMPILER" == "gcc" ]; then
    brew install gcc@7
    export CC=gcc-7
    export CXX=g++-7
fi

if [ "$COMPILER" == "clang" ]; then
    export CC=clang
    export CXX=clang++
fi

set +v
