#!/bin/bash
BUILD_DIR="build/"

if [ ! -d "$BUILD_DIR" ]; then
	mkdir build
fi

mkdir log

cd build
cp ../makefile .
cp ../One_box.sh .
cp ../Two_box.sh .
cp ../Timing.sh .

make

make single
make double
make timing
