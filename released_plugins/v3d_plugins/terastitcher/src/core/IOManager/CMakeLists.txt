# CmakeLists.txt in iomanager dir

# add used modules to include path
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)

# find plugins
SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/plugins)

# ask for selection of the plugins to be included in the project
FOREACH(subdir ${SUBDIRS})
	option (WITH_IO_PLUGIN_${subdir} "Include this plugin in the project" ON) 
ENDFOREACH()

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