project(meta-feedback)

add_library(meta-feedback rocchio.cpp ide_dec_hi.cpp feedback_factory.cpp feedback.cpp)
target_link_libraries(meta-feedback meta-index)

install(TARGETS meta-feedback
        EXPORT meta-exports
        DESTINATION lib)
