#!/bin/zsh

# Quick wrapper script around rgbds to make assembling
# test assembly quicker
rgbasm $1.asm -o $1.o
rgblink $1.o -o $1.gb
rgbfix $1.gb -o $1.gb

hexdump -C $1.gb
