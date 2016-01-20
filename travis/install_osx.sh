set -v
brew update
brew install icu4c
brew outdated cmake || brew upgrade cmake

if [ "$COMPILER" == "gcc" ]; then
    brew install gcc
fi
set +v
