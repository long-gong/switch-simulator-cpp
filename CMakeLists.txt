cmake_minimum_required(VERSION 3.0)

project(switch-simulator)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -w -O3")
include(CheckIncludeFileCXX)

include_directories(common)
include_directories(build_dependencies/include)
set(LEMON_LIB ${CMAKE_CURRENT_SOURCE_DIR}/build_dependencies/lib/libemon.a)

CHECK_INCLUDE_FILE_CXX("lemon/list_graph.h" HAVE_LEMON)
if(NOT ${HAVE_FILE})
    message(fatal_error "lemon c++ was not found")
endif()

include_directories(common/HdrHistogram_c/src)
set(HDR_SRC ${CMAKE_CURRENT_SOURCE_DIR}/common/HdrHistogram_c/src/hdr_histogram.c)
add_library(HDR_OBJ OBJECT ${HDR_SRC})

# Algorithms proposed in literature (More details please refer to README.md)
add_subdirectory(iLQF)
add_subdirectory(O1)
add_subdirectory(iSLIP)
add_subdirectory(Serena)
add_subdirectory(iSLIP_ShakeUp)
add_subdirectory(iLQF_ShakeUp)
add_subdirectory(MWM)

# Algorithms proposed in this work
add_subdirectory(QPS_iSLIP)
add_subdirectory(QPS_Serena)

add_subdirectory(QPP_QPA_iSLIP)
add_subdirectory(QPP_QPA_Serena)

add_subdirectory(FQPS_iSLIP)
add_subdirectory(FQPS_Serena)

