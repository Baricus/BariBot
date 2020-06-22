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
# To build a test suite, "make buildTests" compiles
# the binary and "make test" runs the program.  
# All tests are stored in the TESTDIR folder,
# as designated below.  To ensure that tests can
# use implementations from the final program, it
# depends on (and links) all object files that are
# not "main.o".  Thus, the main function of BariBot
# MUST be in "main.cpp", as main will be created twice
# otherwise.
#
# For development, a number of phony rules are
# defined below to automatically run the program
# with various debugging utilities

# ==============================================
# Variable Assignment/File Management
# ==============================================
CXX = g++

# for gdb add -ggdb and remove -03
CXXFLAGS = -ggdb -Wall -Wextra -std=c++11

# lpthread linked for asio compatability
# lPoco* for HTTP post requests
CXXLIBS  = -lpthread \
		   -lPocoNet -lPocoFoundation -lPocoNetSSL \
		   -lPocoJSON \

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
	$(RM) $(BINARY) $(TSTBINARY) $(OBJECTDIR)* $(DEPENDDIR)*

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
	@$(CXX) $(CXXFLAGS) $(CXXLIBS) $+ -o $(BINARY)

# Makes test binary (depends on all standard objects but main)
$(TSTBINARY): $(TSTOBJS) $(filter-out $(OBJECTDIR)main.o, $(OBJECTS))
	@echo
	@echo Linking tests!
	@$(CXX) $(CXXFLAGS) $(CXXLIBS) $+ -o $(TSTBINARY)

# implicit .cpp file to .o file
# generates dependencies on an object-to-object
# basis at compile time using the "-M" flags
# 
# As we have two directories with code, 
# we create two copies of the rule below,
# one for each working directory, using eval

define define_compile_rules
$(OBJECTDIR)%.o: $(1)%.cpp
	@echo
	@echo Compiling $$<
	@$(CXX) $(CXXFLAGS) -MMD -MP -MF $(DEPENDDIR)$$*.d -c $$< -o $$@
endef

$(eval $(call define_compile_rules, $(SOURCEDIR)))
$(eval $(call define_compile_rules, $(TESTDIR)))

# makes folders if needed
$(OBJECTDIR): 
	mkdir $(OBJECTDIR)

$(DEPENDDIR):
	mkdir $(DEPENDDIR)

#include dependancies
-include $(DEPENDS)
-include $(TSTDEPS)
