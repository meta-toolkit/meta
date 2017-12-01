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
include src/topics/tools/CMakeFiles/lda.dir/depend.make

# Include the progress variables for this target.
include src/topics/tools/CMakeFiles/lda.dir/progress.make

# Include the compile flags for this target's objects.
include src/topics/tools/CMakeFiles/lda.dir/flags.make

src/topics/tools/CMakeFiles/lda.dir/lda.cpp.o: src/topics/tools/CMakeFiles/lda.dir/flags.make
src/topics/tools/CMakeFiles/lda.dir/lda.cpp.o: ../src/topics/tools/lda.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/topics/tools/CMakeFiles/lda.dir/lda.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/topics/tools && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/lda.dir/lda.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/topics/tools/lda.cpp

src/topics/tools/CMakeFiles/lda.dir/lda.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/lda.dir/lda.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/topics/tools && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/topics/tools/lda.cpp > CMakeFiles/lda.dir/lda.cpp.i

src/topics/tools/CMakeFiles/lda.dir/lda.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/lda.dir/lda.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/topics/tools && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/topics/tools/lda.cpp -o CMakeFiles/lda.dir/lda.cpp.s

src/topics/tools/CMakeFiles/lda.dir/lda.cpp.o.requires:

.PHONY : src/topics/tools/CMakeFiles/lda.dir/lda.cpp.o.requires

src/topics/tools/CMakeFiles/lda.dir/lda.cpp.o.provides: src/topics/tools/CMakeFiles/lda.dir/lda.cpp.o.requires
	$(MAKE) -f src/topics/tools/CMakeFiles/lda.dir/build.make src/topics/tools/CMakeFiles/lda.dir/lda.cpp.o.provides.build
.PHONY : src/topics/tools/CMakeFiles/lda.dir/lda.cpp.o.provides

src/topics/tools/CMakeFiles/lda.dir/lda.cpp.o.provides.build: src/topics/tools/CMakeFiles/lda.dir/lda.cpp.o


# Object files for target lda
lda_OBJECTS = \
"CMakeFiles/lda.dir/lda.cpp.o"

# External object files for target lda
lda_EXTERNAL_OBJECTS =

lda: src/topics/tools/CMakeFiles/lda.dir/lda.cpp.o
lda: src/topics/tools/CMakeFiles/lda.dir/build.make
lda: lib/libmeta-topics.a
lda: lib/libmeta-index.a
lda: lib/libmeta-analyzers.a
lda: lib/libmeta-corpus.a
lda: lib/libmeta-filters.a
lda: lib/libmeta-io.a
lda: lib/libmeta-util.a
lda: /usr/lib/libz.dylib
lda: /usr/local/lib/liblzma.dylib
lda: lib/libmeta-tokenizers.a
lda: lib/libmeta-utf.dylib
lda: lib/libmeta-eval.a
lda: /usr/lib/libc++.dylib
lda: /usr/local/lib/libjemalloc.dylib
lda: /usr/lib/libdl.dylib
lda: /usr/lib/libc++abi.dylib
lda: src/topics/tools/CMakeFiles/lda.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../lda"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/topics/tools && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/lda.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/topics/tools/CMakeFiles/lda.dir/build: lda

.PHONY : src/topics/tools/CMakeFiles/lda.dir/build

src/topics/tools/CMakeFiles/lda.dir/requires: src/topics/tools/CMakeFiles/lda.dir/lda.cpp.o.requires

.PHONY : src/topics/tools/CMakeFiles/lda.dir/requires

src/topics/tools/CMakeFiles/lda.dir/clean:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/topics/tools && $(CMAKE_COMMAND) -P CMakeFiles/lda.dir/cmake_clean.cmake
.PHONY : src/topics/tools/CMakeFiles/lda.dir/clean

src/topics/tools/CMakeFiles/lda.dir/depend:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mihikadave/Documents/CS510_MP/meta /Users/mihikadave/Documents/CS510_MP/meta/src/topics/tools /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/topics/tools /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/topics/tools/CMakeFiles/lda.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/topics/tools/CMakeFiles/lda.dir/depend

