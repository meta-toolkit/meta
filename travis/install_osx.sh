set -v
brew update
brew install icu4c jemalloc
brew outdated cmake || brew upgrade cmake

if [ "$COMPILER" == "gcc" ]; then
    brew tap homebrew/versions
    brew install homebrew/versions/gcc5
    export CC=gcc-5
    export CXX=g++-5
fi

if [ "$COMPILER" == "clang" ]; then
    export CC=clang
    export CXX=clang++
fi

set +v
