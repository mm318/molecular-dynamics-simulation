cmake_minimum_required(VERSION 3.0.0)

include(GNUInstallDirs)

set(PROJECT_NAME molecular_dynamics_sim)
project(${PROJECT_NAME})


#########################
# External Dependencies #
#########################

include(ExternalProject)

ExternalProject_Add(zlib
    URL "${CMAKE_SOURCE_DIR}/external_libs/zlib.tar.bz2"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
)

ExternalProject_Add(libpng
    DEPENDS zlib
    URL "${CMAKE_SOURCE_DIR}/external_libs/libpng.tar.bz2"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
)

ExternalProject_Add(thr_pool
    URL "${CMAKE_SOURCE_DIR}/external_libs/thr_pool.tar.bz2"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
)

link_directories("${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")


###########
# Project #
###########

add_subdirectory("src/2d_sim")
add_subdirectory("src/3d_sim")
