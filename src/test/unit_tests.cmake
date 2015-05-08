add_test(analyzers ${UNIT_TEST_EXE} analyzers)
set_tests_properties(analyzers PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(stemmers ${UNIT_TEST_EXE} stemmers)
set_tests_properties(stemmers PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(parallel ${UNIT_TEST_EXE} parallel)
set_tests_properties(parallel PROPERTIES TIMEOUT 30 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(inverted-index ${UNIT_TEST_EXE} inverted-index)
set_tests_properties(inverted-index PROPERTIES TIMEOUT 30 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(forward-index ${UNIT_TEST_EXE} forward-index)
set_tests_properties(forward-index PROPERTIES TIMEOUT 30 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(string-list ${UNIT_TEST_EXE} string-list)
set_tests_properties(string-list PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(vocabulary-map ${UNIT_TEST_EXE} vocabulary-map)
set_tests_properties(vocabulary-map PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(libsvm-parser ${UNIT_TEST_EXE} libsvm-parser)
set_tests_properties(libsvm-parser PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(classifiers ${UNIT_TEST_EXE} classifiers)
set_tests_properties(classifiers PROPERTIES TIMEOUT 80 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(rankers ${UNIT_TEST_EXE} rankers)
set_tests_properties(rankers PROPERTIES TIMEOUT 75 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(ir-eval ${UNIT_TEST_EXE} ir-eval)
set_tests_properties(ir-eval PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(compression ${UNIT_TEST_EXE} compression)
set_tests_properties(compression PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(graph ${UNIT_TEST_EXE} graph)
set_tests_properties(graph PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(parser ${UNIT_TEST_EXE} parser)
set_tests_properties(parser PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

add_test(language-model ${UNIT_TEST_EXE} language-model)
set_tests_properties(language-model PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
