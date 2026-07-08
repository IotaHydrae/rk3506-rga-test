#!/bin/bash
set -e

usage() {
    echo "Usage: ./build.sh [-c|--clean] [-h|--help]"
    echo "  -c, --clean   Clean build directory before building"
    echo "  -h, --help    Show this help"
    exit 0
}

CLEAN=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        -c|--clean) CLEAN=1; shift ;;
        -h|--help)  usage ;;
        *)          echo "Unknown option: $1"; usage ;;
    esac
done

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

TOOLCHAIN_PATH="${SCRIPT_DIR}/toolchain_linux.cmake"
LIBRGA_PATH="${SCRIPT_DIR}/libs/"
BUILD_DIR="${SCRIPT_DIR}/build"
BUILD_TYPE="${BUILD_TYPE:-Release}"

if [[ $CLEAN -eq 1 ]]; then
    rm -rf "${BUILD_DIR}"
fi

mkdir -p "${BUILD_DIR}"

cmake -B "${BUILD_DIR}" \
    -DLIBRGA_FILE_LIB="${LIBRGA_PATH}" \
    -DBUILD_TOOLCHAINS_PATH="${TOOLCHAIN_PATH}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_INSTALL_PREFIX=install \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake --build "${BUILD_DIR}" -j$(nproc)
