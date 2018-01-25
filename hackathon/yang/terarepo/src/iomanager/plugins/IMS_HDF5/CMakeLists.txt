# find all headers and sources
file(GLOB headers *.h)
file(GLOB sources *.cpp)

# add parent dir to include path
include_directories (../../)

# get current directory name
get_filename_component(dirname ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# add static library
add_library(ioplugin_${dirname} STATIC ${headers} ${sources})
set_target_properties(ioplugin_${dirname} PROPERTIES POSITION_INDEPENDENT_CODE ON)

# include 3rd party libraries
include_directories (${CMAKE_SOURCE_DIR}/3rdparty/zlib)
include_directories (${CMAKE_BINARY_DIR}/3rdparty/zlib)

# check HDF5 option
option (WITH_HDF5 "Support to HDF5" OFF) 
if(WITH_HDF5)
	message(STATUS "Checking HDF5")
	find_package (HDF5 NAMES hdf5 COMPONENTS C static)
	
	if(HDF5_FOUND)
	
		# include HDF5 headers
		INCLUDE_DIRECTORIES (${HDF5_INCLUDE_DIR})
		
		# enable HDF5-dependent code
		
		# override hdf5 target properties to remove target zlib-target that causes conflicts with zlib already present in this project
		set_target_properties(${HDF5_C_STATIC_LIBRARY} PROPERTIES INTERFACE_LINK_LIBRARIES "szip-static")
		
		# link to HDF5
		target_link_libraries(ioplugin_${dirname} ${HDF5_C_STATIC_LIBRARY})
		
		# on UNIX, link to additional zlib dependencies
		if(UNIX AND (CMAKE_COMPILER_IS_GNUCXX OR CV_ICC))
		  target_link_libraries(ioplugin_${dirname} dl z)
		endif()
	
		# OK
		message(STATUS "Checking HDF5 ... OK!")
		
	else(HDF5_FOUND)
		message(FATAL_ERROR "Checking HDF5 ... error!!!")
		
	endif(HDF5_FOUND)
endif(WITH_HDF5)

