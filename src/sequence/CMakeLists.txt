project(meta-sequence)

add_subdirectory(analyzers)
add_subdirectory(crf)
add_subdirectory(tools)

add_library(meta-sequence observation.cpp
                          sequence.cpp
                          sequence_analyzer.cpp
                          trellis.cpp
                          io/ptb_parser.cpp)
target_link_libraries(meta-sequence meta-io meta-utf)

add_library(meta-greedy-tagger perceptron.cpp)
target_link_libraries(meta-greedy-tagger meta-sequence meta-io)

install(TARGETS
          meta-sequence meta-greedy-tagger
        EXPORT
          meta-exports
        DESTINATION
          lib)
