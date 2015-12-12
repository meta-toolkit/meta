add_test(analyzers ${UNIT_TEST_EXE} --only=[analyzers])
set_tests_properties(analyzers PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(stemmers ${UNIT_TEST_EXE} --only=[stemmers])
set_tests_properties(stemmers PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(parallel ${UNIT_TEST_EXE} --only=[parallel])
set_tests_properties(parallel PROPERTIES TIMEOUT 30 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(inverted-index ${UNIT_TEST_EXE} --only=[inverted-index])
set_tests_properties(inverted-index PROPERTIES TIMEOUT 30 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(forward-index ${UNIT_TEST_EXE} --only=[forward-index])
set_tests_properties(forward-index PROPERTIES TIMEOUT 30 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(classifier ${UNIT_TEST_EXE} --only=[classifier])
set_tests_properties(classifier PROPERTIES TIMEOUT 100 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(string-list ${UNIT_TEST_EXE} --only=[string-list])
set_tests_properties(string-list PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(vocabulary-map ${UNIT_TEST_EXE} --only=[vocabulary-map])
set_tests_properties(vocabulary-map PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(libsvm-parser ${UNIT_TEST_EXE} --only=[libsvm-parser])
set_tests_properties(libsvm-parser PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(rankers ${UNIT_TEST_EXE} --only=[rankers])
set_tests_properties(rankers PROPERTIES TIMEOUT 90 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(ir-eval ${UNIT_TEST_EXE} --only=[ir-eval])
set_tests_properties(ir-eval PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(binary-io ${UNIT_TEST_EXE} --only=[binary-io])
set_tests_properties(binary-io PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(graph ${UNIT_TEST_EXE} --only=[graph])
set_tests_properties(graph PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(parser ${UNIT_TEST_EXE} --only=[parser])
set_tests_properties(parser PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(language-model ${UNIT_TEST_EXE} --only=[language-model])
set_tests_properties(language-model PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(filesystem ${UNIT_TEST_EXE} --only=[filesystem])
set_tests_properties(filesystem PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(feature-selection ${UNIT_TEST_EXE} --only=[feature-selection])
set_tests_properties(feature-selection PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(tokenizer-filter ${UNIT_TEST_EXE} --only=[tokenizer-filter])
set_tests_properties(tokenizer-filter PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
