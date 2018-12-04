cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME NeuronStructNavigator)
# Output Variables
set(OUTPUT_DEBUG Debug/bin)
set(OUTPUT_REL ./)
# Folders files
set(CPP_DIR_1 ../../../../v3d_external/v3d_main/basic_c_fun)
set(CPP_DIR_2 ../../../../v3d_external/v3d_main/neuron_editing)
set(CPP_DIR_3 ./)
set(CPP_DIR_4 ./)
set(HEADER_DIR_1 ../../../../v3d_external/v3d_main/basic_c_fun)
set(HEADER_DIR_2 ../../../../v3d_external/v3d_main/neuron_editing)
set(HEADER_DIR_3 )

############## CMake Project ################
#        The main options of project        #
#############################################

project(${PROJECT_NAME} CXX)

# Define Release by default.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
  message(STATUS "Build type not specified: Use Release by default.")
endif(NOT CMAKE_BUILD_TYPE)

# Definition of Macros
add_definitions(
   -D_DEBUG 
   -D_LIB 
   -DUNICODE
   -D_UNICODE
)

############## Artefacts Output #################
# Defines outputs , depending Debug or Release. #
#################################################

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
  set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
  set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
endif()

################# Flags ################
# Defines Flags for Windows and Linux. #
########################################

if(MSVC)
   set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3 /MD /Od /Zi /EHsc")
   set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W3 /GL /Od /Oi /Gy /Zi /EHsc")
endif(MSVC)
if(NOT MSVC)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
   if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
   endif()
endif(NOT MSVC)

################ Files ################
#   --   Add files to project.   --   #
#######################################

file(GLOB SRC_FILES
    ${CPP_DIR_1}/*.cpp
    ${CPP_DIR_2}/*.cpp
    ${CPP_DIR_3}/*.cpp
    ${CPP_DIR_4}/*.cpp
    ${HEADER_DIR_1}/*.h
    ${HEADER_DIR_2}/*.h
    ${HEADER_DIR_3}/*.h
)

# Add library to build.
add_library(${PROJECT_NAME} STATIC
   ${SRC_FILES}
)

