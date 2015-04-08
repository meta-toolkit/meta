message(STATUS "Setting up biicode block...")
set(UNIT_TEST_EXE skystrife_meta_src_test_tools_unit-test)
include(src/test/unit_tests.cmake)
ADD_BIICODE_TARGETS()
target_link_libraries(${BII_LIB_TARGET} PUBLIC meta-definitions
                                               ${ICU_LIBRARIES}
                                               ${ZLIB_LIBRARIES}
                                               ${CMAKE_THREAD_LIB_INIT})
