# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.14

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
CMAKE_COMMAND = "/cygdrive/c/Users/Michael Olson/.CLion2019.2/system/cygwin_cmake/bin/cmake.exe"

# The command to remove a file.
RM = "/cygdrive/c/Users/Michael Olson/.CLion2019.2/system/cygwin_cmake/bin/cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /cygdrive/d/Documents/C/NewCard

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /cygdrive/d/Documents/C/NewCard/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/NewCard.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/NewCard.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/NewCard.dir/flags.make

CMakeFiles/NewCard.dir/main.c.o: CMakeFiles/NewCard.dir/flags.make
CMakeFiles/NewCard.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/cygdrive/d/Documents/C/NewCard/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/NewCard.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/NewCard.dir/main.c.o   -c /cygdrive/d/Documents/C/NewCard/main.c

CMakeFiles/NewCard.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/NewCard.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /cygdrive/d/Documents/C/NewCard/main.c > CMakeFiles/NewCard.dir/main.c.i

CMakeFiles/NewCard.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/NewCard.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /cygdrive/d/Documents/C/NewCard/main.c -o CMakeFiles/NewCard.dir/main.c.s

# Object files for target NewCard
NewCard_OBJECTS = \
"CMakeFiles/NewCard.dir/main.c.o"

# External object files for target NewCard
NewCard_EXTERNAL_OBJECTS =

NewCard.exe: CMakeFiles/NewCard.dir/main.c.o
NewCard.exe: CMakeFiles/NewCard.dir/build.make
NewCard.exe: CMakeFiles/NewCard.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/cygdrive/d/Documents/C/NewCard/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable NewCard.exe"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/NewCard.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/NewCard.dir/build: NewCard.exe

.PHONY : CMakeFiles/NewCard.dir/build

CMakeFiles/NewCard.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/NewCard.dir/cmake_clean.cmake
.PHONY : CMakeFiles/NewCard.dir/clean

CMakeFiles/NewCard.dir/depend:
	cd /cygdrive/d/Documents/C/NewCard/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /cygdrive/d/Documents/C/NewCard /cygdrive/d/Documents/C/NewCard /cygdrive/d/Documents/C/NewCard/cmake-build-debug /cygdrive/d/Documents/C/NewCard/cmake-build-debug /cygdrive/d/Documents/C/NewCard/cmake-build-debug/CMakeFiles/NewCard.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/NewCard.dir/depend

