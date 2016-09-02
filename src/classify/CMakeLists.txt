project(meta-classify)

add_subdirectory(kernel)
add_subdirectory(tools)

ExternalProject_Add(liblinear
  SOURCE_DIR ${meta_SOURCE_DIR}/../deps/libsvm-modules/liblinear
  BINARY_DIR ${meta_SOURCE_DIR}/../deps/libsvm-modules/liblinear/build
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
  CMAKE_CACHE_ARGS "-DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}"
                   "-DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}"
  INSTALL_COMMAND "")

ExternalProject_Add(libsvm
  SOURCE_DIR ${meta_SOURCE_DIR}/../deps/libsvm-modules/libsvm
  BINARY_DIR ${meta_SOURCE_DIR}/../deps/libsvm-modules/libsvm/build
  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release
  CMAKE_CACHE_ARGS "-DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}"
                   "-DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}"
  INSTALL_COMMAND "")

add_library(meta-classify binary_classifier_factory.cpp
                          classifier/binary_classifier.cpp
                          classifier/classifier.cpp
                          classifier/dual_perceptron.cpp
                          classifier/knn.cpp
                          classifier/nearest_centroid.cpp
                          classifier/logistic_regression.cpp
                          classifier/naive_bayes.cpp
                          classifier/one_vs_all.cpp
                          classifier/one_vs_one.cpp
                          classifier/sgd.cpp
                          classifier/svm_wrapper.cpp
                          classifier/winnow.cpp
                          classifier_factory.cpp
                          confusion_matrix.cpp)
target_link_libraries(meta-classify meta-ranker meta-learn meta-kernel)
add_dependencies(meta-classify liblinear libsvm)

install(TARGETS meta-classify
        EXPORT meta-exports
        DESTINATION lib)
