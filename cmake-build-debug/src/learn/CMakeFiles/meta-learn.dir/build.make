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
include src/learn/CMakeFiles/meta-learn.dir/depend.make

# Include the progress variables for this target.
include src/learn/CMakeFiles/meta-learn.dir/progress.make

# Include the compile flags for this target's objects.
include src/learn/CMakeFiles/meta-learn.dir/flags.make

src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.o: src/learn/CMakeFiles/meta-learn.dir/flags.make
src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.o: ../src/learn/sgd.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/learn && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta-learn.dir/sgd.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/learn/sgd.cpp

src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta-learn.dir/sgd.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/learn && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/learn/sgd.cpp > CMakeFiles/meta-learn.dir/sgd.cpp.i

src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta-learn.dir/sgd.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/learn && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/learn/sgd.cpp -o CMakeFiles/meta-learn.dir/sgd.cpp.s

src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.o.requires:

.PHONY : src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.o.requires

src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.o.provides: src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.o.requires
	$(MAKE) -f src/learn/CMakeFiles/meta-learn.dir/build.make src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.o.provides.build
.PHONY : src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.o.provides

src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.o.provides.build: src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.o


# Object files for target meta-learn
meta__learn_OBJECTS = \
"CMakeFiles/meta-learn.dir/sgd.cpp.o"

# External object files for target meta-learn
meta__learn_EXTERNAL_OBJECTS =

lib/libmeta-learn.a: src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.o
lib/libmeta-learn.a: src/learn/CMakeFiles/meta-learn.dir/build.make
lib/libmeta-learn.a: src/learn/CMakeFiles/meta-learn.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library ../../lib/libmeta-learn.a"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/learn && $(CMAKE_COMMAND) -P CMakeFiles/meta-learn.dir/cmake_clean_target.cmake
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/learn && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/meta-learn.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/learn/CMakeFiles/meta-learn.dir/build: lib/libmeta-learn.a

.PHONY : src/learn/CMakeFiles/meta-learn.dir/build

src/learn/CMakeFiles/meta-learn.dir/requires: src/learn/CMakeFiles/meta-learn.dir/sgd.cpp.o.requires

.PHONY : src/learn/CMakeFiles/meta-learn.dir/requires

src/learn/CMakeFiles/meta-learn.dir/clean:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/learn && $(CMAKE_COMMAND) -P CMakeFiles/meta-learn.dir/cmake_clean.cmake
.PHONY : src/learn/CMakeFiles/meta-learn.dir/clean

src/learn/CMakeFiles/meta-learn.dir/depend:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mihikadave/Documents/CS510_MP/meta /Users/mihikadave/Documents/CS510_MP/meta/src/learn /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/learn /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/learn/CMakeFiles/meta-learn.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/learn/CMakeFiles/meta-learn.dir/depend

