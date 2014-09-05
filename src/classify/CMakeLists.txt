project(meta-classify)

add_subdirectory(loss)
add_subdirectory(tools)

add_library(meta-classify binary_classifier_factory.cpp
                          classifier/binary_classifier.cpp
                          classifier/classifier.cpp
                          classifier/dual_perceptron.cpp
                          classifier/knn.cpp
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
