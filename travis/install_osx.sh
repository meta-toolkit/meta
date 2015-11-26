set -v
brew update
brew install icu4c
brew outdated cmake || brew upgrade cmake
set +v
