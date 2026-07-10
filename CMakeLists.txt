cmake_minimum_required(VERSION 3.12)

if (EXISTS ${BUILD_TOOLCHAINS_PATH})
    message("load ${BUILD_TOOLCHAINS_PATH}")
    include(${BUILD_TOOLCHAINS_PATH})
endif()

if (EXISTS ${LIBRGA_FILE_LIB}/librga.so)
    message("load ${LIBRGA_FILE_LIB}/librga.so")
    set(RGA_LIB ${LIBRGA_FILE_LIB}/librga.so)
else ()
    set(RGA_LIB rga)
endif()

get_filename_component(TARGET_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
project(rga_${TARGET_NAME})

if (NOT DEFINED CMAKE_INSTALL_BINDIR)
    set(CMAKE_INSTALL_BINDIR bin)
endif()

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -Wunused-function -Wunused-variable -Wl,--allow-shlib-undefined")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wunused-function -Wunused-variable -std=c++11 -Wl,--allow-shlib-undefined")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/allocator/include
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/3rdparty/libdrm/include
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/3rdparty/libdrm/include/libdrm
)

string(REPLACE "-DANDROID" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# utils static library — shared by all test programs
add_library(rga_utils STATIC
    utils/utils.cpp
    utils/allocator/drm_alloc.cpp
    utils/allocator/dma_alloc.cpp
)

# auto-discover example programs under examples/
file(GLOB TEST_SOURCES examples/*.cpp)
foreach(TEST_SRC ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SRC})
    target_link_libraries(${TEST_NAME} rga_utils ${RGA_LIB} drm dl)
endforeach()
