# BariBot makefile - Miles Shamo 2020
#
# This is a makefile to compile BariBot, 
# but is likely able to compile any program if
# the same directory structure is used as it
# is intended to be fully configurable.
# 
# This makefile defaults to the "build" target. 
# If immediate execution is desired, "make run" 
# is implemented.
#
# For development, a number of phony rules are
# defined below to automatically run the program
# with various debugging utilities

# ==============================================
# Variable Assignment/File Management
# ==============================================
CXX = g++

# for gdb add -ggdb and remove -03
CXXFLAGS = -O3 -Wall -Wextra
BINARY = BariBot

# File structure
SOURCEDIR = ./source/
OBJECTDIR = ./objects/
DEPENDDIR = ./depends/

# if a goal other than build is desired as default, change this
.DEFAULT_GOAL = build

# Below this point it is likely nothing ever need be changed

# gets files
SOURCES:=$(wildcard $(SOURCEDIR)*.cpp)
OBJECTS:=$(patsubst $(SOURCEDIR)%.cpp, $(OBJECTDIR)%.o, $(SOURCES))
DEPENDS:=$(patsubst $(SOURCEDIR)%.cpp, $(DEPENDDIR)%.d, $(SOURCES))

# ==============================================
# Phony Rules
# ==============================================

# out of personal prefference I clear the terminal
# on most specialized builds (run, gdb, etc)
run: $(BINARY)
	@clear
	@./$(BINARY)

gdb: $(BINARY)
	@clear
	@gdb $(BINARY)

valgrind: $(BINARY)
	@clear
	@valgrind ./$(BINARY) -leak-check=full

clean:
	@echo Cleaning up!
	$(RM) $(BINARY) $(OBJECTDIR)* $(DEPENDDIR)*

build: $(BINARY)
	@echo
	@echo Binary is Built!

# Debug rules to output file lists to ensure
# all files are properly accounted for
printSources:
	@echo $(SOURCES)

printObjects:
	@echo $(OBJECTS)

printDepends:
	@echo $(DEPENDS)

.PHONY: run gdb valgrind clean build printSources printObjects printDepends

# ==============================================
# Compilation rules
# ==============================================

# makes binary
$(BINARY): $(OBJECTS)
	@echo
	@echo Linking objects!
	@$(CXX) $(CXXFLAGS) $+ -o $(BINARY)

# implicit .cpp file to .o file
# generates dependencies on an object-to-object
# basis at compile time using the "-M" flags
$(OBJECTDIR)%.o: $(SOURCEDIR)%.cpp
	@echo
	@echo Compiling $<
	@$(CXX) $(CXXFLAGS) -MMD -MP -MF $(DEPENDDIR)$*.d -c $< -o $@

#include dependancies
-include $(DEPENDS)
