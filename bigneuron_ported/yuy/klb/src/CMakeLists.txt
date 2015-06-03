#KLB stands for Keller Lab Block image format
project ( imageBlockAPI_project )
cmake_minimum_required ( VERSION 2.8 )

#folder where all the executables go (this is seens by all elements called using add_subdirectory)
SET(KLB_BINARY_FOLDER "${PROJECT_SOURCE_DIR}/../bin")


#add fpic for non MSVC compilers
IF( NOT MSVC )
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -pthread")
ENDIF()

#add bzip2 library
include_directories ("${PROJECT_SOURCE_DIR}/external/bzip2")
add_subdirectory ("${PROJECT_SOURCE_DIR}/external/bzip2")
LINK_DIRECTORIES( "${CMAKE_CURRENT_BINARY_DIR}/external/bzip2" )

#add zlib library
include_directories ("${PROJECT_SOURCE_DIR}/external/zlib")
add_subdirectory ("${PROJECT_SOURCE_DIR}/external/zlib")
LINK_DIRECTORIES( "${CMAKE_CURRENT_BINARY_DIR}/external/zlib" )
include_directories( "${CMAKE_CURRENT_BINARY_DIR}/external/zlib" ) #for zconf.h

#main sources and headers
file(GLOB KLB_SRCS ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB KLB_HDRS ${PROJECT_SOURCE_DIR}/*.h)


#main library 
IF(NOT TARGET klblib)
ADD_LIBRARY(klblib ${KLB_SRCS} ${KLB_HDRS})
target_link_libraries (klblib bz2lib zlibstatic)
ENDIF()

#main shared library (DLL)
IF(NOT TARGET klblibs)
ADD_LIBRARY(klblibs SHARED ${KLB_SRCS} ${KLB_HDRS})
target_link_libraries (klblibs bz2lib zlibstatic)
set_property(TARGET klblibs PROPERTY COMPILE_DEFINITIONS COMPILE_SHARED_LIBRARY)
set_property(TARGET klblibs PROPERTY OUTPUT_NAME klb)
ENDIF()

#executable to test klblib library for I/O purposes
IF(NOT TARGET test_klbIO)
ADD_EXECUTABLE(test_klbIO mainTest_klbIO.cxx)
target_link_libraries (test_klbIO klblib)
ENDIF()

#executable to test klblib library for I/O purposes using C-style code
IF(NOT TARGET test_klbIO_C)
ADD_EXECUTABLE(test_klbIO_C mainTest_klbIO.c common.h klb_Cwrapper.h)
target_link_libraries (test_klbIO_C klblibs)
ENDIF()

#executable to test klblib library for speed and size in real data
IF( NOT TARGET speed_klbIO)
ADD_EXECUTABLE(speed_klbIO mainTestSpeed_klbIO.cxx)
target_link_libraries (speed_klbIO klblib)
ENDIF()

IF( CMAKE_COMPILER_IS_GNUCC )
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") #to allow C+11 features in older GCC compilers
ENDIF()

IF( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" ) 
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++") #to allow C+11 features in older GCC compilers
ENDIF()

#small executable to test C++11 std::threads and compare it to pbzip2
IF( NOT TARGET testPbzip2_Cpp11Threads)
ADD_EXECUTABLE(testPbzip2_Cpp11Threads testPbzip2_Cpp11Threads.cxx)
target_link_libraries ( testPbzip2_Cpp11Threads bz2lib zlibstatic)
ENDIF()

option(BINDINGS_JAVA "Build bindings for Java" off)
if(BINDINGS_JAVA)
    find_package(SWIG REQUIRED)
    find_package(JNI REQUIRED)
    include(${SWIG_USE_FILE})

    set_source_files_properties(klb.i PROPERTIES CPLUSPLUS ON)
    set(CMAKE_SWIG_FLAGS -package org.janelia.simview.klb.jni)
	set(CMAKE_SWIG_OUTDIR ${PROJECT_BINARY_DIR}/java/org/janelia/simview/klb/jni)

    include_directories (${PROJECT_SOURCE_DIR} ${JNI_INCLUDE_DIRS})
    link_directories(${PROJECT_BINARY_DIR} ${JNI_LIBRARY_DIRS})
    swig_add_module(klb-jni java klb.i)
    swig_link_libraries(klb-jni klblibs)
endif()


#copy binaries to specific folder for release version
INSTALL(TARGETS  klblib klblibs test_klbIO test_klbIO_C DESTINATION "${KLB_BINARY_FOLDER}" CONFIGURATIONS Release)
