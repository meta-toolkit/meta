if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    cmake -DCMAKE_BUILD_TYPE=$1 ..
fi

if [ "$TRAVIS_OS_NAME" == "osx" ]; then
    cmake -DCMAKE_BUILD_TYPE=$1 -DICU_ROOT=/usr/local/opt/icu4c ..
fi
