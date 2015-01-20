add_executable(profile profile.cpp)
target_link_libraries(profile meta-index
                              ${CMAKE_THREAD_LIBS_INIT})
