cmake_minimum_required(VERSION 2.4)

PROJECT(supervoxel)
INCLUDE_REGULAR_EXPRESSION("^.*$")

#set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../modules")

#SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_BUILD_TYPE Release)
#SET(EXECUTABLE_OUTPUT_PATH ~/bin/EM
#    CACHE PATH "Single output directory for building all executables.")

set(CMAKE_CXX_FLAGS         "")
set(CMAKE_CXX_FLAGS_DEBUG   "-g -pg")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

set(CMAKE_C_FLAGS         ${CMAKE_CXX_FLAGS})
set(CMAKE_C_FLAGS_DEBUG   ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})

#FIND_PACKAGE(ITK REQUIRED PATHS /data/phd/software/itk/compile/InsightToolkit-3.20.0)

FIND_PACKAGE(OpenCV REQUIRED)
# add include directories
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS} ${ITK_INCLUDE_DIRS})
LINK_DIRECTORIES(${ITK_LIBRARY_DIRS})
add_library(supervoxel
LKM.cpp
utils.cpp)
TARGET_LINK_LIBRARIES(supervoxel ${OpenCV_LIBS})
SET_TARGET_PROPERTIES(supervoxel PROPERTIES COMPILE_FLAGS -fPIC)

ADD_EXECUTABLE(supervoxel_test
supervoxel_test.cpp
LKM.cpp
utils.cpp)
TARGET_LINK_LIBRARIES(supervoxel_test ${OpenCV_LIBS})

ADD_EXECUTABLE(superpixel_test
superpixel_test.cpp
LKM.cpp
utils.cpp)
TARGET_LINK_LIBRARIES(superpixel_test ${OpenCV_LIBS})
