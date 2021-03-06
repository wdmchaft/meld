#!/bin/sh
#
# Creates a Makefile for building Meld.
#

cp Makefile.in Makefile
echo "Finding dependencies..."
makedepend -- -- `find . -name '*.cpp'` > /dev/null 2>&1
rm -f Makefile.bak
echo "Makefile built - type 'make' to compile project"
