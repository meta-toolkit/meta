# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.9

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/mihikadave/Documents/CS510_MP/meta

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug

# Include any dependencies generated for this target.
include src/tools/CMakeFiles/top-k.dir/depend.make

# Include the progress variables for this target.
include src/tools/CMakeFiles/top-k.dir/progress.make

# Include the compile flags for this target's objects.
include src/tools/CMakeFiles/top-k.dir/flags.make

src/tools/CMakeFiles/top-k.dir/top_k.cpp.o: src/tools/CMakeFiles/top-k.dir/flags.make
src/tools/CMakeFiles/top-k.dir/top_k.cpp.o: ../src/tools/top_k.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/tools/CMakeFiles/top-k.dir/top_k.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/tools && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/top-k.dir/top_k.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/tools/top_k.cpp

src/tools/CMakeFiles/top-k.dir/top_k.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/top-k.dir/top_k.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/tools && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/tools/top_k.cpp > CMakeFiles/top-k.dir/top_k.cpp.i

src/tools/CMakeFiles/top-k.dir/top_k.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/top-k.dir/top_k.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/tools && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/tools/top_k.cpp -o CMakeFiles/top-k.dir/top_k.cpp.s

src/tools/CMakeFiles/top-k.dir/top_k.cpp.o.requires:

.PHONY : src/tools/CMakeFiles/top-k.dir/top_k.cpp.o.requires

src/tools/CMakeFiles/top-k.dir/top_k.cpp.o.provides: src/tools/CMakeFiles/top-k.dir/top_k.cpp.o.requires
	$(MAKE) -f src/tools/CMakeFiles/top-k.dir/build.make src/tools/CMakeFiles/top-k.dir/top_k.cpp.o.provides.build
.PHONY : src/tools/CMakeFiles/top-k.dir/top_k.cpp.o.provides

src/tools/CMakeFiles/top-k.dir/top_k.cpp.o.provides.build: src/tools/CMakeFiles/top-k.dir/top_k.cpp.o


# Object files for target top-k
top__k_OBJECTS = \
"CMakeFiles/top-k.dir/top_k.cpp.o"

# External object files for target top-k
top__k_EXTERNAL_OBJECTS =

top-k: src/tools/CMakeFiles/top-k.dir/top_k.cpp.o
top-k: src/tools/CMakeFiles/top-k.dir/build.make
top-k: lib/libmeta-index.a
top-k: lib/libmeta-analyzers.a
top-k: lib/libmeta-corpus.a
top-k: lib/libmeta-filters.a
top-k: lib/libmeta-io.a
top-k: lib/libmeta-util.a
top-k: /usr/lib/libz.dylib
top-k: /usr/local/lib/liblzma.dylib
top-k: lib/libmeta-tokenizers.a
top-k: lib/libmeta-utf.dylib
top-k: lib/libmeta-eval.a
top-k: /usr/lib/libc++.dylib
top-k: /usr/local/lib/libjemalloc.dylib
top-k: /usr/lib/libdl.dylib
top-k: /usr/lib/libc++abi.dylib
top-k: src/tools/CMakeFiles/top-k.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../top-k"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/tools && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/top-k.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/tools/CMakeFiles/top-k.dir/build: top-k

.PHONY : src/tools/CMakeFiles/top-k.dir/build

src/tools/CMakeFiles/top-k.dir/requires: src/tools/CMakeFiles/top-k.dir/top_k.cpp.o.requires

.PHONY : src/tools/CMakeFiles/top-k.dir/requires

src/tools/CMakeFiles/top-k.dir/clean:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/tools && $(CMAKE_COMMAND) -P CMakeFiles/top-k.dir/cmake_clean.cmake
.PHONY : src/tools/CMakeFiles/top-k.dir/clean

src/tools/CMakeFiles/top-k.dir/depend:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mihikadave/Documents/CS510_MP/meta /Users/mihikadave/Documents/CS510_MP/meta/src/tools /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/tools /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/tools/CMakeFiles/top-k.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/tools/CMakeFiles/top-k.dir/depend

