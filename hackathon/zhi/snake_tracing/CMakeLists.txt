cmake_minimum_required(VERSION 2.4)
if(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
endif(COMMAND CMAKE_POLICY)

project(snake_tracing)

find_package(Qt4 REQUIRED)
include( ${QT_USE_FILE} )

find_package(Boost)
include_directories ( ${Boost_INCLUDE_DIRS})

include(CTest)

# include(CPack) # save cpack for parent directory

find_path(V3D_BINARY_DIR v3d)
set(INSTALLATION_DIRECTORY ${V3D_BINARY_DIR}/plugins)

find_path(V3D_BASIC_C_FUN_SOURCE_DIR v3d_interface.h)
if( V3D_BASIC_C_FUN_SOURCE_DIR )
  set( V3D_BASIC_C_FUN_BINARY_DIR  ${PROJECT_BINARY_DIR}/Source/basic_c_fun )
endif( V3D_BASIC_C_FUN_SOURCE_DIR )

if(NOT V3D_BINARY_DIR)
  message(FATAL "The V3D_BINARY_DIR is required. This must point to the location of the v3d executable")
endif()


set (LIBRARY_OUTPUT_PATH /local1/work/v3d_external/bin/plugins/snake_tracing
    CACHE INTERNAL "Output directory for the shared libraries.")

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin
    CACHE INTERNAL "Output directory for the executables.")


include_directories( ${V3D_BASIC_C_FUN_SOURCE_DIR} )
#include_directories( /local2/neuron_tracing_comparision/farsight-src/ftkImage )

# CMB Nov-11-2010 Need to wait until all plugins are built before running macdeployqt
add_custom_target(FinishedPlugins
    COMMENT "All plugins have been built")

include(${PROJECT_SOURCE_DIR}/CMake/PluginConfiguration.cmake)

#Find ITK
find_package(ITK REQUIRED)
include( ${ITK_USE_FILE} )

#Find VTK
find_package(VTK REQUIRED)
include( ${VTK_USE_FILE} )

#Find VXL
find_package(VXL REQUIRED)
set(VXL_PROVIDE_STANDARD_optionS 1)
include(${VXL_CMAKE_DIR}/UseVXL.cmake)
 

set(PLUGIN_SOURCES snake_tracing_plugin.cpp
		   TracingCore/ImageOperation.cpp
    			TracingCore/PointOperation.cpp	
    			TracingCore/SnakeOperation.cpp
			TracingCore/OpenSnakeTracer.cpp
			#ParametersGroup.cpp
		  ${V3D_BASIC_C_FUN_SOURCE_DIR}/v3d_message.cpp)

Set(PLUGIN_HEADERS	snake_tracing_plugin.h
			TracingCore/PointOperation.h
		        Rendering/ImageActors.h
    			TracingCore/ImageOperation.h
    			TracingCore/SnakeOperation.h
			TracingCore/OpenSnakeTracer.h)
set(PLUGIN_DIRECTORY_NAME snake_tracing)
link_libraries( ${ITK_LIBRARIES})
#set( LIB_DIR /local2/neuron_tracing_comparision/farsight_bin/libs)
#link_libraries( ${LIB_DIR})
#Target_Link_Libraries(${LIB_DIR}/libftkImage.a)
#SET(TracingSystem_LIBRARIES
#  ${QT_LIBRARIES} TraceCoreLibrary ftkCommon ${TracingSysLibraries} TinyXML)


configure_v3d_plugin(snake_tracing)

