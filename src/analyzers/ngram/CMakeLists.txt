project(meta-ngram-analyzers)

add_library(meta-ngram-analyzers ngram_analyzer.cpp
                                 ngram_lex_analyzer.cpp
                                 ngram_pos_analyzer.cpp
                                 ngram_simple_analyzer.cpp
                                 ngram_word_analyzer.cpp)
