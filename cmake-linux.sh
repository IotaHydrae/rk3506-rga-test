#!/bin/bash

SCRIPT_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
SAMPLES_DIR=${SCRIPT_DIR}/

# The following options require configuration
TOOLCHAIN_PATH=${SAMPLES_DIR}/toolchain_linux.cmake
LIBRGA_PATH=${SAMPLES_DIR}/libs/
BUILD_DIR=build/build_linux
BUILD_TYPE=Release

rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR
pushd $BUILD_DIR

cmake ../.. \
	-DLIBRGA_FILE_LIB=${LIBRGA_PATH} \
	-DBUILD_TOOLCHAINS_PATH=${TOOLCHAIN_PATH} \
	-DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
	-DCMAKE_INSTALL_PREFIX=install \
	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \

make -j8
make install

popd
