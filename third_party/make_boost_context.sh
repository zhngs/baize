#!/bin/bash

BOOST=boost_1_66_0
BOOST_PATH=https://boostorg.jfrog.io/artifactory/main/release/1.66.0/source/$BOOST.tar.gz

download() {
    if [ -f "$1" ]; then
        echo "$1 existed."
    else
        echo "$1 not existed, begin to download..."
        wget $2
        if [ $? -eq 0 ]; then
            echo "download $1 successed";
        else
            echo "Error: download $1 failed";
            return 1;
        fi
    fi
    return 0
}

build_boost_context()
{
  rm -rf $BOOST.tar.gz
  download $BOOST.tar.gz $BOOST_PATH
  tar -xvf $BOOST.tar.gz
  cd $BOOST
  ./bootstrap.sh
  # build boost-context only
  ./b2 --with-context variant=release
}

build_boost_context
