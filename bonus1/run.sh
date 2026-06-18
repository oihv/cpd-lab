#!/bin/bash

clang++ -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Isrc -g  src/gui.cpp -o build/gui 
./build/gui
