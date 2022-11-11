#!/bin/bash

set -x

SOURCE_DIR=`pwd`
BUILD_DIR=$SOURCE_DIR/build

# for clang-tidy
ln -sf $BUILD_DIR/compile_commands.json

# DCMAKE_EXPORT_COMPILE_COMMANDS=ON for compile_commands.json
mkdir -p $BUILD_DIR \
  && cd $BUILD_DIR \
  && cmake \
           -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
           $SOURCE_DIR \
  && make $*
