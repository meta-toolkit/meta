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
include src/embeddings/tools/CMakeFiles/meta-to-glove.dir/depend.make

# Include the progress variables for this target.
include src/embeddings/tools/CMakeFiles/meta-to-glove.dir/progress.make

# Include the compile flags for this target's objects.
include src/embeddings/tools/CMakeFiles/meta-to-glove.dir/flags.make

src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o: src/embeddings/tools/CMakeFiles/meta-to-glove.dir/flags.make
src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o: ../src/embeddings/tools/meta_to_glove.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/embeddings/tools && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/embeddings/tools/meta_to_glove.cpp

src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/embeddings/tools && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/embeddings/tools/meta_to_glove.cpp > CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.i

src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/embeddings/tools && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/embeddings/tools/meta_to_glove.cpp -o CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.s

src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o.requires:

.PHONY : src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o.requires

src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o.provides: src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o.requires
	$(MAKE) -f src/embeddings/tools/CMakeFiles/meta-to-glove.dir/build.make src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o.provides.build
.PHONY : src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o.provides

src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o.provides.build: src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o


# Object files for target meta-to-glove
meta__to__glove_OBJECTS = \
"CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o"

# External object files for target meta-to-glove
meta__to__glove_EXTERNAL_OBJECTS =

meta-to-glove: src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o
meta-to-glove: src/embeddings/tools/CMakeFiles/meta-to-glove.dir/build.make
meta-to-glove: lib/libmeta-util.a
meta-to-glove: lib/libmeta-io.a
meta-to-glove: lib/libmeta-util.a
meta-to-glove: /usr/lib/libc++.dylib
meta-to-glove: /usr/local/lib/libjemalloc.dylib
meta-to-glove: /usr/lib/libz.dylib
meta-to-glove: /usr/local/lib/liblzma.dylib
meta-to-glove: /usr/lib/libdl.dylib
meta-to-glove: /usr/lib/libc++abi.dylib
meta-to-glove: src/embeddings/tools/CMakeFiles/meta-to-glove.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../meta-to-glove"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/embeddings/tools && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/meta-to-glove.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/embeddings/tools/CMakeFiles/meta-to-glove.dir/build: meta-to-glove

.PHONY : src/embeddings/tools/CMakeFiles/meta-to-glove.dir/build

src/embeddings/tools/CMakeFiles/meta-to-glove.dir/requires: src/embeddings/tools/CMakeFiles/meta-to-glove.dir/meta_to_glove.cpp.o.requires

.PHONY : src/embeddings/tools/CMakeFiles/meta-to-glove.dir/requires

src/embeddings/tools/CMakeFiles/meta-to-glove.dir/clean:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/embeddings/tools && $(CMAKE_COMMAND) -P CMakeFiles/meta-to-glove.dir/cmake_clean.cmake
.PHONY : src/embeddings/tools/CMakeFiles/meta-to-glove.dir/clean

src/embeddings/tools/CMakeFiles/meta-to-glove.dir/depend:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mihikadave/Documents/CS510_MP/meta /Users/mihikadave/Documents/CS510_MP/meta/src/embeddings/tools /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/embeddings/tools /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/embeddings/tools/CMakeFiles/meta-to-glove.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/embeddings/tools/CMakeFiles/meta-to-glove.dir/depend

