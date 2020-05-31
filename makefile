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
# lpthread linked for asio compatability
CXXFLAGS = -O3 -Wall -Wextra -std=c++11 -lpthread
BINARY = BariBot
TSTBINARY = TEST.out

# File structure
SOURCEDIR = ./source/
TESTDIR = ./tests/

OBJECTDIR = ./objects/
DEPENDDIR = ./depends/

# if a goal other than build is desired as default, change this
.DEFAULT_GOAL = build

# Below this point it is likely nothing ever need be changed

# gets files
SOURCES:=$(wildcard $(SOURCEDIR)*.cpp)
OBJECTS:=$(patsubst $(SOURCEDIR)%.cpp, $(OBJECTDIR)%.o, $(SOURCES))
DEPENDS:=$(patsubst $(SOURCEDIR)%.cpp, $(DEPENDDIR)%.d, $(SOURCES))

TSTSRCS:=$(wildcard $(TESTDIR)*.cpp)
TSTOBJS:=$(patsubst $(TESTDIR)%.cpp, $(OBJECTDIR)%.o, $(TSTSRCS))
TSTDEPS:=$(patsubst $(TESTDIR)%.cpp, $(DEPENDDIR)%.d, $(TSTSRCS))

# ==============================================
# Phony Rules
# ==============================================

# out of personal prefference I clear the terminal
# on most specialized builds (run, gdb, etc)
run: build
	@clear
	@./$(BINARY)

test: buildTests
	@clear
	@./$(TSTBINARY)

gdb: $(BINARY)
	@clear
	@gdb $(BINARY)

valgrind: $(BINARY)
	@clear
	@valgrind ./$(BINARY) -leak-check=full

clean:
	@echo Cleaning up!
	$(RM) $(BINARY) $(OBJECTDIR)* $(DEPENDDIR)*

build: $(OBJECTDIR) $(DEPENDDIR) $(BINARY)
	@echo
	@echo Binary is built!

buildTests: $(OJBECTDIR) $(DEPENDDIR) $(TSTBINARY)
	@echo
	@echo Tests are built!

# Debug rules to output file lists to ensure
# all files are properly accounted for
printSources:
	@echo $(SOURCES)

printTests:
	@echo $(TSTSRCS)

printObjects:
	@echo $(OBJECTS)

printDepends:
	@echo $(DEPENDS)

.PHONY: run test gdb valgrind clean build buildTests printSources printObjects printDepends printTests

# ==============================================
# Compilation rules
# ==============================================

# makes binary
$(BINARY): $(OBJECTS)
	@echo
	@echo Linking objects!
	@$(CXX) $(CXXFLAGS) $+ -o $(BINARY)

# Makes test binary (depends on all c++ files to allow cross refference)
$(TSTBINARY): $(TSTOBJS) $(OBJECTS)
	@echo
	@echo Linking tests!
	@$(CXX) $(CXXFLAGS) $+ -o $(TSTBINARY)

# implicit .cpp file to .o file
# generates dependencies on an object-to-object
# basis at compile time using the "-M" flags
$(OBJECTDIR)%.o: $(SOURCEDIR)%.cpp
	@echo
	@echo Compiling $<
	@$(CXX) $(CXXFLAGS) -MMD -MP -MF $(DEPENDDIR)$*.d -c $< -o $@

# makes folders if needed
$(OBJECTDIR): 
	mkdir $(OBJECTDIR)

$(DEPENDDIR):
	mkdir $(DEPENDDIR)

#include dependancies
-include $(DEPENDS)
-include $(TSTDEPS)
