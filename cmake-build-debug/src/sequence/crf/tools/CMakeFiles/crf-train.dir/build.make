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
include src/sequence/crf/tools/CMakeFiles/crf-train.dir/depend.make

# Include the progress variables for this target.
include src/sequence/crf/tools/CMakeFiles/crf-train.dir/progress.make

# Include the compile flags for this target's objects.
include src/sequence/crf/tools/CMakeFiles/crf-train.dir/flags.make

src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.o: src/sequence/crf/tools/CMakeFiles/crf-train.dir/flags.make
src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.o: ../src/sequence/crf/tools/crf_train.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/sequence/crf/tools && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/crf-train.dir/crf_train.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/sequence/crf/tools/crf_train.cpp

src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/crf-train.dir/crf_train.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/sequence/crf/tools && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/sequence/crf/tools/crf_train.cpp > CMakeFiles/crf-train.dir/crf_train.cpp.i

src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/crf-train.dir/crf_train.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/sequence/crf/tools && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/sequence/crf/tools/crf_train.cpp -o CMakeFiles/crf-train.dir/crf_train.cpp.s

src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.o.requires:

.PHONY : src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.o.requires

src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.o.provides: src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.o.requires
	$(MAKE) -f src/sequence/crf/tools/CMakeFiles/crf-train.dir/build.make src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.o.provides.build
.PHONY : src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.o.provides

src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.o.provides.build: src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.o


# Object files for target crf-train
crf__train_OBJECTS = \
"CMakeFiles/crf-train.dir/crf_train.cpp.o"

# External object files for target crf-train
crf__train_EXTERNAL_OBJECTS =

crf-train: src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.o
crf-train: src/sequence/crf/tools/CMakeFiles/crf-train.dir/build.make
crf-train: lib/libmeta-crf.a
crf-train: lib/libmeta-sequence.a
crf-train: lib/libmeta-io.a
crf-train: lib/libmeta-util.a
crf-train: /usr/lib/libz.dylib
crf-train: /usr/local/lib/liblzma.dylib
crf-train: lib/libmeta-utf.dylib
crf-train: /usr/lib/libc++.dylib
crf-train: /usr/local/lib/libjemalloc.dylib
crf-train: /usr/lib/libdl.dylib
crf-train: /usr/lib/libc++abi.dylib
crf-train: src/sequence/crf/tools/CMakeFiles/crf-train.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../../crf-train"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/sequence/crf/tools && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/crf-train.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/sequence/crf/tools/CMakeFiles/crf-train.dir/build: crf-train

.PHONY : src/sequence/crf/tools/CMakeFiles/crf-train.dir/build

src/sequence/crf/tools/CMakeFiles/crf-train.dir/requires: src/sequence/crf/tools/CMakeFiles/crf-train.dir/crf_train.cpp.o.requires

.PHONY : src/sequence/crf/tools/CMakeFiles/crf-train.dir/requires

src/sequence/crf/tools/CMakeFiles/crf-train.dir/clean:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/sequence/crf/tools && $(CMAKE_COMMAND) -P CMakeFiles/crf-train.dir/cmake_clean.cmake
.PHONY : src/sequence/crf/tools/CMakeFiles/crf-train.dir/clean

src/sequence/crf/tools/CMakeFiles/crf-train.dir/depend:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mihikadave/Documents/CS510_MP/meta /Users/mihikadave/Documents/CS510_MP/meta/src/sequence/crf/tools /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/sequence/crf/tools /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/sequence/crf/tools/CMakeFiles/crf-train.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/sequence/crf/tools/CMakeFiles/crf-train.dir/depend

