project(meta-classify)

add_subdirectory(loss)
add_subdirectory(tools)

ExternalProject_Add(liblinear
  SOURCE_DIR ${meta_SOURCE_DIR}/../deps/libsvm-modules/liblinear
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make
  LOG_BUILD 0
  INSTALL_COMMAND "")

ExternalProject_Add(libsvm
  SOURCE_DIR ${meta_SOURCE_DIR}/../deps/libsvm-modules/libsvm
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND ""
  BUILD_COMMAND make
  LOG_BUILD 0
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
target_link_libraries(meta-classify meta-index meta-loss)
add_dependencies(meta-classify liblinear libsvm)
