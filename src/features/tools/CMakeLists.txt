add_executable(feature-summary feature_summary.cpp)
target_link_libraries(feature-summary meta-features meta-index
    meta-sequence-analyzers meta-parser-analyzers)
