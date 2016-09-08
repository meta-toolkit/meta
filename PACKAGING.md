# Packaging or Installing MeTA

Please follow these slightly modified build instructions if you are either
a packager and you want to make a package of MeTA for your distribution, or
if you simply want to install MeTA to e.g. `/usr/local`.

You will need the following `cmake` flags:

- `-DBUILD_SHARED_LIBS=On`: build `.so` libraries instead of `.a`.
- `-DBUILD_STATIC_ICU=On`: force building a static ICU library as part of
    `meta-utf.so` to ensure Unicode standard stability.

If you are building for a Linux platform, you *must* use GCC. **Do not use
Clang/libc++**, as the library throws exceptions and must be built in the
same way all other C++ applications on the system are expected to be built.

MeTA will require [cpptoml][cpptoml] to be installed. This is
straightforward as cpptoml is header-only. Please follow the standard
`cmake` build instructions for cpptoml and package using `make install` to
install the CMake configuration files for cpptoml. This allows MeTA's CMake
configuration files to find cpptoml as a dependency.

MeTA can then be installed using `make install`.

# Using an Installed MeTA

You can consume MeTA most easily by using a CMake build system (though you
do not have to). Your `CMakeLists.txt` might look something like the
following:

```cmake
find_package(MeTA 2.4 REQUIRED)

add_executable(my-program my_program.cpp)
target_link_libraries(my-program meta-index) # or any other MeTA libraries
```

[cpptoml]: https://github.com/skystrife/cpptoml
