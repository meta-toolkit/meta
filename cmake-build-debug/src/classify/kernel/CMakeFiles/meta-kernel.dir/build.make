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
include src/classify/kernel/CMakeFiles/meta-kernel.dir/depend.make

# Include the progress variables for this target.
include src/classify/kernel/CMakeFiles/meta-kernel.dir/progress.make

# Include the compile flags for this target's objects.
include src/classify/kernel/CMakeFiles/meta-kernel.dir/flags.make

src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o: src/classify/kernel/CMakeFiles/meta-kernel.dir/flags.make
src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o: ../src/classify/kernel/kernel_factory.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/classify/kernel/kernel_factory.cpp

src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta-kernel.dir/kernel_factory.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/classify/kernel/kernel_factory.cpp > CMakeFiles/meta-kernel.dir/kernel_factory.cpp.i

src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta-kernel.dir/kernel_factory.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/classify/kernel/kernel_factory.cpp -o CMakeFiles/meta-kernel.dir/kernel_factory.cpp.s

src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o.requires:

.PHONY : src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o.requires

src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o.provides: src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o.requires
	$(MAKE) -f src/classify/kernel/CMakeFiles/meta-kernel.dir/build.make src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o.provides.build
.PHONY : src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o.provides

src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o.provides.build: src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o


src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.o: src/classify/kernel/CMakeFiles/meta-kernel.dir/flags.make
src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.o: ../src/classify/kernel/polynomial.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta-kernel.dir/polynomial.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/classify/kernel/polynomial.cpp

src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta-kernel.dir/polynomial.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/classify/kernel/polynomial.cpp > CMakeFiles/meta-kernel.dir/polynomial.cpp.i

src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta-kernel.dir/polynomial.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/classify/kernel/polynomial.cpp -o CMakeFiles/meta-kernel.dir/polynomial.cpp.s

src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.o.requires:

.PHONY : src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.o.requires

src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.o.provides: src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.o.requires
	$(MAKE) -f src/classify/kernel/CMakeFiles/meta-kernel.dir/build.make src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.o.provides.build
.PHONY : src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.o.provides

src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.o.provides.build: src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.o


src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.o: src/classify/kernel/CMakeFiles/meta-kernel.dir/flags.make
src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.o: ../src/classify/kernel/radial_basis.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta-kernel.dir/radial_basis.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/classify/kernel/radial_basis.cpp

src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta-kernel.dir/radial_basis.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/classify/kernel/radial_basis.cpp > CMakeFiles/meta-kernel.dir/radial_basis.cpp.i

src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta-kernel.dir/radial_basis.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/classify/kernel/radial_basis.cpp -o CMakeFiles/meta-kernel.dir/radial_basis.cpp.s

src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.o.requires:

.PHONY : src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.o.requires

src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.o.provides: src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.o.requires
	$(MAKE) -f src/classify/kernel/CMakeFiles/meta-kernel.dir/build.make src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.o.provides.build
.PHONY : src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.o.provides

src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.o.provides.build: src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.o


src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.o: src/classify/kernel/CMakeFiles/meta-kernel.dir/flags.make
src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.o: ../src/classify/kernel/sigmoid.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.o"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && /Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/meta-kernel.dir/sigmoid.cpp.o -c /Users/mihikadave/Documents/CS510_MP/meta/src/classify/kernel/sigmoid.cpp

src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/meta-kernel.dir/sigmoid.cpp.i"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/mihikadave/Documents/CS510_MP/meta/src/classify/kernel/sigmoid.cpp > CMakeFiles/meta-kernel.dir/sigmoid.cpp.i

src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/meta-kernel.dir/sigmoid.cpp.s"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && /Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/mihikadave/Documents/CS510_MP/meta/src/classify/kernel/sigmoid.cpp -o CMakeFiles/meta-kernel.dir/sigmoid.cpp.s

src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.o.requires:

.PHONY : src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.o.requires

src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.o.provides: src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.o.requires
	$(MAKE) -f src/classify/kernel/CMakeFiles/meta-kernel.dir/build.make src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.o.provides.build
.PHONY : src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.o.provides

src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.o.provides.build: src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.o


# Object files for target meta-kernel
meta__kernel_OBJECTS = \
"CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o" \
"CMakeFiles/meta-kernel.dir/polynomial.cpp.o" \
"CMakeFiles/meta-kernel.dir/radial_basis.cpp.o" \
"CMakeFiles/meta-kernel.dir/sigmoid.cpp.o"

# External object files for target meta-kernel
meta__kernel_EXTERNAL_OBJECTS =

lib/libmeta-kernel.a: src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o
lib/libmeta-kernel.a: src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.o
lib/libmeta-kernel.a: src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.o
lib/libmeta-kernel.a: src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.o
lib/libmeta-kernel.a: src/classify/kernel/CMakeFiles/meta-kernel.dir/build.make
lib/libmeta-kernel.a: src/classify/kernel/CMakeFiles/meta-kernel.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX static library ../../../lib/libmeta-kernel.a"
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && $(CMAKE_COMMAND) -P CMakeFiles/meta-kernel.dir/cmake_clean_target.cmake
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/meta-kernel.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/classify/kernel/CMakeFiles/meta-kernel.dir/build: lib/libmeta-kernel.a

.PHONY : src/classify/kernel/CMakeFiles/meta-kernel.dir/build

src/classify/kernel/CMakeFiles/meta-kernel.dir/requires: src/classify/kernel/CMakeFiles/meta-kernel.dir/kernel_factory.cpp.o.requires
src/classify/kernel/CMakeFiles/meta-kernel.dir/requires: src/classify/kernel/CMakeFiles/meta-kernel.dir/polynomial.cpp.o.requires
src/classify/kernel/CMakeFiles/meta-kernel.dir/requires: src/classify/kernel/CMakeFiles/meta-kernel.dir/radial_basis.cpp.o.requires
src/classify/kernel/CMakeFiles/meta-kernel.dir/requires: src/classify/kernel/CMakeFiles/meta-kernel.dir/sigmoid.cpp.o.requires

.PHONY : src/classify/kernel/CMakeFiles/meta-kernel.dir/requires

src/classify/kernel/CMakeFiles/meta-kernel.dir/clean:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel && $(CMAKE_COMMAND) -P CMakeFiles/meta-kernel.dir/cmake_clean.cmake
.PHONY : src/classify/kernel/CMakeFiles/meta-kernel.dir/clean

src/classify/kernel/CMakeFiles/meta-kernel.dir/depend:
	cd /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/mihikadave/Documents/CS510_MP/meta /Users/mihikadave/Documents/CS510_MP/meta/src/classify/kernel /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel /Users/mihikadave/Documents/CS510_MP/meta/cmake-build-debug/src/classify/kernel/CMakeFiles/meta-kernel.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/classify/kernel/CMakeFiles/meta-kernel.dir/depend

