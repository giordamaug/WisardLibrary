cmake_minimum_required(VERSION 2.8)
# Change compiler to your prefereti compier
set( CMAKE_C_COMPILER /opt/local/bin/gcc-mp-5 )
set( CMAKE_CXX_COMPILER /opt/local/bin/g++-mp-5 )

project( wisardlib-cxx_3.0 )

set(LIBRARY_NAME wisard-cxx_3.0)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${LIBRARY_NAME} SHARED wisard.cpp wnet_lib.cpp)
get_target_property(sf ${LIBRARY_NAME} IMPORT_SUFFIX)
install(FILES libwisard-cxx_3.0${CMAKE_SHARED_LIBRARY_SUFFIX} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries( ${LIBRARY_NAME})
# Compile test
add_executable(test test.cpp )
target_link_libraries (test ${LIBRARY_NAME})
