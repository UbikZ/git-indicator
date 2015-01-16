#!/bin/sh
# Script which provides a exemple of simple build of libgit w/o cmake variable
# but with manual copy

[ ! -d $1 ] || [ "$1" = "" ] && echo "Path has to exist and be a directory" \
&& exit 0

rm -rf $1/build
mkdir $1/build && cd $1/build
cmake ..
cmake --build .
sudo cp -d libgit2.so* /usr/lib/x86_64-linux-gnu/
sudo cp -r ../include/* /usr/include/
cd .. && rm -rf ./build
