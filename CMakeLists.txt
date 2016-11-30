cmake_minimum_required(VERSION 3.0)
# Change compiler to GCC to support OpenMP
if( USE_GCC )
set( CMAKE_C_COMPILER /opt/local/bin/gcc-mp-4.9 )
set( CMAKE_CXX_COMPILER /opt/local/bin/g++-mp-4.9 )
message(Building\ with\ GCC\ to\ include\ OpenMP\ support)
else()
set(CMAKE_CXX_FLAGS "-stdlib=libstdc++ ${CMAKE_CXX_FLAGS}")
message(Building\ with\ CLANG\) )
endif()

project( wisardlib-cxx_3.0 )


set(LIBRARY_NAME wisard-cxx_3.0)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(${LIBRARY_NAME} SHARED wisard.cpp wnet_lib.cpp)
get_target_property(sf ${LIBRARY_NAME} IMPORT_SUFFIX)
install(FILES libwisard-cxx_3.0${CMAKE_SHARED_LIBRARY_SUFFIX} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries( ${LIBRARY_NAME} ${OpenCV_LIBS})
