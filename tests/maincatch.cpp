/* maincatch.cpp - Miles Shamo
 *
 * The catch2 framework is very handy for running
 * unit tests.  However, compiling it's main function
 * is rather time consuming.  As such, we isolate
 * the catch main in this file so that it can be
 * compiled independantly from all unit tests.
 *
 */

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

