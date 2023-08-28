cmake_minimum_required(VERSION 3.0.0)

include(GNUInstallDirs)

set(PROJECT_NAME mdsim_2d)
project(${PROJECT_NAME})

set(SOURCES
    2d_motion.c
    2d_position.c
    2d_sim_main.c
    2d_visual.c
    ../utils/my_rand.c
    ../utils/quadtree.c
    ../utils/png_writer.c
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

target_include_directories(${PROJECT_NAME} BEFORE
    PRIVATE "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_INCLUDEDIR}"
            "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(${PROJECT_NAME}
    libthr_pool.a
    pthread
    GL
    glut
    liblibpng.a
    libzlib.a
    m
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
