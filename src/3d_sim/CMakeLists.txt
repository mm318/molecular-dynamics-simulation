cmake_minimum_required(VERSION 3.0.0)

include(GNUInstallDirs)

set(PROJECT_NAME mdsim_3d)
project(${PROJECT_NAME})

set(SOURCES
    3d_motion.c
    3d_position.c
    3d_sim_main.c
    3d_visual.c
    ../utils/my_rand.c
    ../utils/octree.c
    ../utils/png_writer.c
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

add_dependencies(${PROJECT_NAME}
    thr_pool
    libpng
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
