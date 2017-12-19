BUILD_CMD="${BUILD_CMD:-make -j2}"
TEST_CMD="${TEST_CMD:-./unit-test --reporter=spec}"

cmake -DCMAKE_BUILD_TYPE=Release $CMAKE_EXTRA_ARGS .. && $BUILD_CMD && $TEST_CMD
