project(meta-loss)

add_library(meta-loss hinge.cpp
                      huber.cpp
                      least_squares.cpp
                      logistic.cpp
                      loss_function_factory.cpp
                      modified_huber.cpp
                      perceptron.cpp
                      smooth_hinge.cpp
                      squared_hinge.cpp)
