#!/bin/zsh

# Quick script to build and test gbemu without leaving the testing
# directory
pushd ../build; ninja; popd; meson test -v -C ../build
