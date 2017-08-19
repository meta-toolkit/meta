project(meta-mman-win32)

# The below is currently marked as SHARED because as of cmake 3.9.1-1 of
# MSYS2, linking against a static library that has C code in it adds
# "-lgcc_eh -lgcc_eh" to the linker line, which causes difficult-to-debug
# (due to corrupted stack frames) segmentation faults when attempting to
# create indexes. Marking it as a shared library and linking against that
# does *not* add the "-lgcc_eh -lgcc_eh" to the linker line, and the issues
# disappear. Since these lines weren't added prior to cmake 3.9.1-1, there
# were no problems with the previous approach before then.
add_library(meta-mman-win32 SHARED mman.c)
target_link_libraries(meta-mman-win32 meta-definitions)

install(TARGETS meta-mman-win32
        EXPORT meta-exports
        DESTINATION lib)
