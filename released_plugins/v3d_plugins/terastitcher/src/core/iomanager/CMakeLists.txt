# CmakeLists.txt in iomanager dir

# add used modules to include path
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)

# find plugins
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/plugins)

# ask for selection of the plugins to be included in the project
# tiff2D and tiff3D are mandatory, thus they are set to ON by default
FOREACH(subdir ${SUBDIRS})
	if(subdir STREQUAL "tiff2D" OR subdir STREQUAL "tiff3D")
		option (WITH_IO_PLUGIN_${subdir} "Include this plugin (mandatory) in the project" ON)
	else()
		option (WITH_IO_PLUGIN_${subdir} "Include this plugin (optional) in the project" OFF)
	endif()	 
ENDFOREACH()

# check if tiff2D and tiff3D are selected. If they are not, an error is thrown
if(NOT WITH_IO_PLUGIN_tiff2D)
	message(SEND_ERROR "You cannot deselect plugin tiff2D (mandatory)")
endif()
if(NOT WITH_IO_PLUGIN_tiff3D)
	message(SEND_ERROR "You cannot deselect plugin tiff3D (mandatory)")
endif()

# check if opencv2D is selected. If it's not, we hide the OpenCV_DIR variable from the GUI
if(NOT WITH_IO_PLUGIN_opencv2D)
	set ( OpenCV_DIR CACHE INTERNAL "not set" FORCE )
endif()

# include selected plugins only
FOREACH(subdir ${SUBDIRS})
	if(WITH_IO_PLUGIN_${subdir})
		message(STATUS "Include I/O plugin " "${subdir} ...")
		add_subdirectory(plugins/${subdir})
		message(STATUS "Include I/O plugin " "${subdir} ... OK!")
	endif(WITH_IO_PLUGIN_${subdir})
ENDFOREACH()

# create header that includes all plugin headers (assuming they are are named like their plugin directory)
file(WRITE ioplugins.h "// Include plugins headers\n")
FOREACH(subdir ${SUBDIRS})
	if(WITH_IO_PLUGIN_${subdir})
		file(APPEND ioplugins.h "#include \"plugins/${subdir}/${subdir}.h\"\n")
	endif(WITH_IO_PLUGIN_${subdir})
ENDFOREACH()

# add STATIC library from the all .h and .cpp files
file(GLOB iomanager_headers *.h)
file(GLOB iomanager_sources *.cpp)
add_library(iomanager STATIC ${iomanager_headers} ${iomanager_sources} ${plugin_headers} ${plugin_sources})

# link with the selected plugins
FOREACH(subdir ${SUBDIRS})
	if(WITH_IO_PLUGIN_${subdir})
		target_link_libraries(iomanager ioplugin_${subdir})
	endif(WITH_IO_PLUGIN_${subdir})
ENDFOREACH()

# link with other dependencies
target_link_libraries(iomanager imagemanager )
