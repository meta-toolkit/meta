add_executable(graph-test graph_test.cpp)
target_link_libraries(graph-test meta-util ${CMAKE_THREAD_LIBS_INIT})
if(META_HAS_EXPERIMENTAL_FILESYSTEM)
  target_link_libraries(graph-test c++experimental)
endif()

add_executable(wiki-page-rank wiki_page_rank.cpp)
target_link_libraries(wiki-page-rank cpptoml meta-io meta-util
    ${CMAKE_THREAD_LIBS_INIT})
if(META_HAS_EXPERIMENTAL_FILESYSTEM)
  target_link_libraries(wiki-page-rank c++experimental)
endif()
