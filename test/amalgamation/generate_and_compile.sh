#!/bin/bash

#############################################################
# Copyright (C) 2026 Alexander Borisov
#
# Author: Alexander Borisov <borisov@lexbor.com>
#
# This file is part of Lexbor.
##############################################################

# Exits if any command fails
set -e

# The Perl script to run
SCRIPT="../../single.pl"

# Check if the script exists
if [ ! -f "$SCRIPT" ]; then
    echo "Error: $SCRIPT does not exist."
    exit 1
fi

# Get the list of modules
echo "Fetching modules list..."
MODULES=$($SCRIPT --modules)

# Compilation flags
CFLAGS=${CFLAGS:="-Wall -Werror -pedantic -pipe -std=c99 -fPIC"}
CC=${CC:-gcc}

echo "Modules to process:"
echo "$MODULES"

for MOD in $MODULES; do
    echo "----------------------------------------------------------------"
    echo "Processing module: $MOD"

    SINGLE_FILE="${MOD}.h"

    # Generate the C code (header) for the module
    $SCRIPT "$MOD" > "$SINGLE_FILE"

    # Check if the file was created
    if [ ! -s "$SINGLE_FILE" ]; then
        echo "Error: Generated file $SINGLE_FILE is empty or missing."
        exit 1
    fi

    # Compile the header file
    # Used -x c-header to explicitly treat the input as a C header file
    echo "Compiling $SINGLE_FILE..."
    if $CC $CFLAGS -x c-header "$SINGLE_FILE" -o /dev/null; then
        echo "Success: $MOD compiles without errors and warnings."
    else
        echo "Error: $MOD failed to compile."
        rm -f "$SINGLE_FILE"
        exit 1
    fi

    TEST_FILE="code/${MOD}.c"
    TEST_BIN="./test_${MOD}"

    # Check if test code exists
    if [ -f "$TEST_FILE" ]; then
        echo "Compiling and running test for $MOD..."

        # Compile the test file
        # Include current dir for ${MOD}.h and code/ for _base.h
        if $CC $CFLAGS -I. -Icode "$TEST_FILE" -o "$TEST_BIN"; then
             echo "Test compilation successful."

             # Run the test
             if $TEST_BIN; then
                 echo "Success: $MOD test passed."
             else
                 echo "Error: $MOD test failed execution."
                 rm -f "$SINGLE_FILE" "$TEST_BIN"
                 exit 1
             fi

             rm -f "$TEST_BIN"
        else
             echo "Error: $MOD test failed to compile."
             rm -f "$SINGLE_FILE"
             exit 1
        fi
    fi

    # Clean up the generated file
    rm -f "$SINGLE_FILE"
done

echo "----------------------------------------------------------------"
echo "All modules processed and compiled successfully."

# Compile all modules together
echo "----------------------------------------------------------------"
echo "----------------------------------------------------------------"
echo "Compiling all modules together..."

ALL_SINGLE_FILE="all_modules.h"

# Generate the combined C code (header) for all modules
$SCRIPT --all > "$ALL_SINGLE_FILE"

# Check if the file was created
if [ ! -s "$ALL_SINGLE_FILE" ]; then
    echo "Error: Generated file $ALL_SINGLE_FILE is empty or missing."
    exit 1
fi

# Compile the combined header file
if $CC $CFLAGS -x c-header "$ALL_SINGLE_FILE" -o /dev/null; then
    echo "Success: All modules compile together without errors and warnings."
else
    echo "Error: All modules failed to compile together."
    rm -f "$ALL_SINGLE_FILE"
    exit 1
fi

rm -f "$ALL_SINGLE_FILE"

echo "----------------------------------------------------------------"
echo "All modules compiled together successfully."
