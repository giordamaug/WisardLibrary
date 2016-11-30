cmake_minimum_required(VERSION 3.0)
# Change compiler to your prefereti compier
#set( CMAKE_C_COMPILER <path-to-your-c-compiler> )
#set( CMAKE_CXX_COMPILER <path-to-your-c++-compiler> )

project( wisardlib-cxx_3.0 )

set(LIBRARY_NAME wisard-cxx_3.0)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${LIBRARY_NAME} SHARED wisard.cpp wnet_lib.cpp)
get_target_property(sf ${LIBRARY_NAME} IMPORT_SUFFIX)
install(FILES libwisard-cxx_3.0${CMAKE_SHARED_LIBRARY_SUFFIX} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries( ${LIBRARY_NAME} ${OpenCV_LIBS})
