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
include src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/depend.make

# Include the progress variables for this target.
include src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/progress.make

# Include the compile flags for this target's objects.
include src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/flags.make

src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o: src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/flags.make
src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o: ../src/parser/analyzers/tree_analyzer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/analyzers && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/parser/analyzers/tree_analyzer.cpp

src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/analyzers && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/parser/analyzers/tree_analyzer.cpp > CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.i

src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/analyzers && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/parser/analyzers/tree_analyzer.cpp -o CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.s

src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o.requires:

.PHONY : src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o.requires

src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o.provides: src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o.requires
	$(MAKE) -f src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/build.make src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o.provides.build
.PHONY : src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o.provides

src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o.provides.build: src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o


# Object files for target meta-parser-analyzers
meta__parser__analyzers_OBJECTS = \
"CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o"

# External object files for target meta-parser-analyzers
meta__parser__analyzers_EXTERNAL_OBJECTS =

lib/libmeta-parser-analyzers.a: src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o
lib/libmeta-parser-analyzers.a: src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/build.make
lib/libmeta-parser-analyzers.a: src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library ../../../lib/libmeta-parser-analyzers.a"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/analyzers && $(CMAKE_COMMAND) -P CMakeFiles/meta-parser-analyzers.dir/cmake_clean_target.cmake
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/analyzers && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/meta-parser-analyzers.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/build: lib/libmeta-parser-analyzers.a

.PHONY : src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/build

src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/requires: src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/tree_analyzer.cpp.o.requires

.PHONY : src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/requires

src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/clean:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/analyzers && $(CMAKE_COMMAND) -P CMakeFiles/meta-parser-analyzers.dir/cmake_clean.cmake
.PHONY : src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/clean

src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/depend:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mihikadave/Documents/CS510_MP/meta /Users/mihikadave/Documents/CS510_MP/meta/src/parser/analyzers /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/analyzers /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/parser/analyzers/CMakeFiles/meta-parser-analyzers.dir/depend

