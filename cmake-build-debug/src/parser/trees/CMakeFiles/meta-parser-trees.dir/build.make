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
include src/parser/trees/CMakeFiles/meta-parser-trees.dir/depend.make

# Include the progress variables for this target.
include src/parser/trees/CMakeFiles/meta-parser-trees.dir/progress.make

# Include the compile flags for this target's objects.
include src/parser/trees/CMakeFiles/meta-parser-trees.dir/flags.make

src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o: src/parser/trees/CMakeFiles/meta-parser-trees.dir/flags.make
src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o: ../src/parser/trees/leaf_node.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/parser/trees/leaf_node.cpp

src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/parser/trees/leaf_node.cpp > CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.i

src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/parser/trees/leaf_node.cpp -o CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.s

src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o.requires:

.PHONY : src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o.requires

src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o.provides: src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o.requires
	$(MAKE) -f src/parser/trees/CMakeFiles/meta-parser-trees.dir/build.make src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o.provides.build
.PHONY : src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o.provides

src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o.provides.build: src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o


src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.o: src/parser/trees/CMakeFiles/meta-parser-trees.dir/flags.make
src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.o: ../src/parser/trees/node.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta-parser-trees.dir/node.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/parser/trees/node.cpp

src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta-parser-trees.dir/node.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/parser/trees/node.cpp > CMakeFiles/meta-parser-trees.dir/node.cpp.i

src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta-parser-trees.dir/node.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/parser/trees/node.cpp -o CMakeFiles/meta-parser-trees.dir/node.cpp.s

src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.o.requires:

.PHONY : src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.o.requires

src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.o.provides: src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.o.requires
	$(MAKE) -f src/parser/trees/CMakeFiles/meta-parser-trees.dir/build.make src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.o.provides.build
.PHONY : src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.o.provides

src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.o.provides.build: src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.o


src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o: src/parser/trees/CMakeFiles/meta-parser-trees.dir/flags.make
src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o: ../src/parser/trees/internal_node.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/parser/trees/internal_node.cpp

src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta-parser-trees.dir/internal_node.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/parser/trees/internal_node.cpp > CMakeFiles/meta-parser-trees.dir/internal_node.cpp.i

src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta-parser-trees.dir/internal_node.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/parser/trees/internal_node.cpp -o CMakeFiles/meta-parser-trees.dir/internal_node.cpp.s

src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o.requires:

.PHONY : src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o.requires

src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o.provides: src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o.requires
	$(MAKE) -f src/parser/trees/CMakeFiles/meta-parser-trees.dir/build.make src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o.provides.build
.PHONY : src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o.provides

src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o.provides.build: src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o


src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o: src/parser/trees/CMakeFiles/meta-parser-trees.dir/flags.make
src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o: ../src/parser/trees/parse_tree.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/parser/trees/parse_tree.cpp

src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/parser/trees/parse_tree.cpp > CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.i

src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/parser/trees/parse_tree.cpp -o CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.s

src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o.requires:

.PHONY : src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o.requires

src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o.provides: src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o.requires
	$(MAKE) -f src/parser/trees/CMakeFiles/meta-parser-trees.dir/build.make src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o.provides.build
.PHONY : src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o.provides

src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o.provides.build: src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o


# Object files for target meta-parser-trees
meta__parser__trees_OBJECTS = \
"CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o" \
"CMakeFiles/meta-parser-trees.dir/node.cpp.o" \
"CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o" \
"CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o"

# External object files for target meta-parser-trees
meta__parser__trees_EXTERNAL_OBJECTS =

lib/libmeta-parser-trees.a: src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o
lib/libmeta-parser-trees.a: src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.o
lib/libmeta-parser-trees.a: src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o
lib/libmeta-parser-trees.a: src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o
lib/libmeta-parser-trees.a: src/parser/trees/CMakeFiles/meta-parser-trees.dir/build.make
lib/libmeta-parser-trees.a: src/parser/trees/CMakeFiles/meta-parser-trees.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX static library ../../../lib/libmeta-parser-trees.a"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && $(CMAKE_COMMAND) -P CMakeFiles/meta-parser-trees.dir/cmake_clean_target.cmake
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/meta-parser-trees.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/parser/trees/CMakeFiles/meta-parser-trees.dir/build: lib/libmeta-parser-trees.a

.PHONY : src/parser/trees/CMakeFiles/meta-parser-trees.dir/build

src/parser/trees/CMakeFiles/meta-parser-trees.dir/requires: src/parser/trees/CMakeFiles/meta-parser-trees.dir/leaf_node.cpp.o.requires
src/parser/trees/CMakeFiles/meta-parser-trees.dir/requires: src/parser/trees/CMakeFiles/meta-parser-trees.dir/node.cpp.o.requires
src/parser/trees/CMakeFiles/meta-parser-trees.dir/requires: src/parser/trees/CMakeFiles/meta-parser-trees.dir/internal_node.cpp.o.requires
src/parser/trees/CMakeFiles/meta-parser-trees.dir/requires: src/parser/trees/CMakeFiles/meta-parser-trees.dir/parse_tree.cpp.o.requires

.PHONY : src/parser/trees/CMakeFiles/meta-parser-trees.dir/requires

src/parser/trees/CMakeFiles/meta-parser-trees.dir/clean:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees && $(CMAKE_COMMAND) -P CMakeFiles/meta-parser-trees.dir/cmake_clean.cmake
.PHONY : src/parser/trees/CMakeFiles/meta-parser-trees.dir/clean

src/parser/trees/CMakeFiles/meta-parser-trees.dir/depend:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mihikadave/Documents/CS510_MP/meta /Users/mihikadave/Documents/CS510_MP/meta/src/parser/trees /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/parser/trees/CMakeFiles/meta-parser-trees.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/parser/trees/CMakeFiles/meta-parser-trees.dir/depend

