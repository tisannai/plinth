#!/bin/sh

mkdir -p build
gcc -Wall -fPIC -O2 -c src/plinth.c -o build/plinth.o
gcc -shared -o build/libplinth.so build/plinth.o
