add_test(analyzers ${UNIT_TEST_EXE} --only=[analyzers] --reporter=spec)
set_tests_properties(analyzers PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(stemmers ${UNIT_TEST_EXE} --only=[stemmers] --reporter=spec)
set_tests_properties(stemmers PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(parallel ${UNIT_TEST_EXE} --only=[parallel] --reporter=spec)
set_tests_properties(parallel PROPERTIES TIMEOUT 30 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(inverted-index ${UNIT_TEST_EXE} --only=[inverted-index] --reporter=spec)
set_tests_properties(inverted-index PROPERTIES TIMEOUT 30 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(forward-index ${UNIT_TEST_EXE} --only=[forward-index] --reporter=spec)
set_tests_properties(forward-index PROPERTIES TIMEOUT 30 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(classifier ${UNIT_TEST_EXE} --only=[classifier] --reporter=spec)
set_tests_properties(classifier PROPERTIES TIMEOUT 100 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(string-list ${UNIT_TEST_EXE} --only=[string-list] --reporter=spec)
set_tests_properties(string-list PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(vocabulary-map ${UNIT_TEST_EXE} --only=[vocabulary-map] --reporter=spec)
set_tests_properties(vocabulary-map PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(libsvm-parser ${UNIT_TEST_EXE} --only=[libsvm-parser] --reporter=spec)
set_tests_properties(libsvm-parser PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(rankers ${UNIT_TEST_EXE} --only=[rankers] --reporter=spec)
set_tests_properties(rankers PROPERTIES TIMEOUT 90 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(ir-eval ${UNIT_TEST_EXE} --only=[ir-eval] --reporter=spec)
set_tests_properties(ir-eval PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(binary-io ${UNIT_TEST_EXE} --only=[binary-io] --reporter=spec)
set_tests_properties(binary-io PROPERTIES TIMEOUT 30 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(graph ${UNIT_TEST_EXE} --only=[graph] --reporter=spec)
set_tests_properties(graph PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(parser ${UNIT_TEST_EXE} --only=[parser] --reporter=spec)
set_tests_properties(parser PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(language-model ${UNIT_TEST_EXE} --only=[language-model] --reporter=spec)
set_tests_properties(language-model PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(filesystem ${UNIT_TEST_EXE} --only=[filesystem] --reporter=spec)
set_tests_properties(filesystem PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(feature-selection ${UNIT_TEST_EXE} --only=[feature-selection] --reporter=spec)
set_tests_properties(feature-selection PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(tokenizer-filter ${UNIT_TEST_EXE} --only=[tokenizer-filter] --reporter=spec)
set_tests_properties(tokenizer-filter PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(metadata ${UNIT_TEST_EXE} --only=[metadata] --reporter=spec)
set_tests_properties(metadata PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(topics ${UNIT_TEST_EXE} --only=[topics] --reporter=spec)
set_tests_properties(topics PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
add_test(hashing ${UNIT_TEST_EXE} --only=[hashing] --reporter=spec)
set_tests_properties(hashing PROPERTIES TIMEOUT 10 WORKING_DIRECTORY
                         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
