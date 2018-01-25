# CMakeLists.txt in imagemanager dir

# add used modules to include path 
include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories (${TeraStitcher_SOURCE_DIR}/volumemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/tinyxml)
include_directories (${TeraStitcher_SOURCE_DIR}/common)
include_directories (${TeraStitcher_SOURCE_DIR}/mappingmanager)

# add 3rd party modules to include path
include_directories (${CMAKE_SOURCE_DIR}/3rdparty/zlib)
include_directories (${CMAKE_BINARY_DIR}/3rdparty/zlib)
include_directories (${CMAKE_SOURCE_DIR}/3rdparty/libtiff)
include_directories (${CMAKE_BINARY_DIR}/3rdparty/libtiff)
include_directories (${TeraStitcher_SOURCE_DIR}/3rdparty/tinyxml)

# add STATIC library from all .h and .cpp files
file(GLOB imagemanager_headers *.h)
file(GLOB imagemanager_sources *.cpp)
add_library(imagemanager STATIC ${imagemanager_headers} ${imagemanager_sources})
set_target_properties(imagemanager PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_library(imagemanager_obj OBJECT ${imagemanager_headers} ${imagemanager_sources})
set_target_properties(imagemanager_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)

# check HDF5 option
option (WITH_HDF5 "Support to HDF5" OFF) 
if(WITH_HDF5)
	message(STATUS "Checking HDF5")
	find_package (HDF5 NAMES hdf5 COMPONENTS C static)
	
	if(HDF5_FOUND)
	
		# include HDF5 headers
		INCLUDE_DIRECTORIES (${HDF5_INCLUDE_DIR})
		
		# enable HDF5-dependent code
		add_definitions(-DENABLE_BDV_HDF5)
		add_definitions(-DENABLE_IMS_HDF5)
		
		# override hdf5 target properties to remove target zlib-target that causes conflicts with zlib already present in this project
		set_target_properties(${HDF5_C_STATIC_LIBRARY} PROPERTIES INTERFACE_LINK_LIBRARIES "szip-static")
		
		# link to HDF5
		target_link_libraries(imagemanager ${HDF5_C_STATIC_LIBRARY})
		
		# on UNIX, link to additional zlib dependencies
		if(UNIX AND (CMAKE_COMPILER_IS_GNUCXX OR CV_ICC))
		  target_link_libraries(imagemanager dl z)
		endif()
	
		# OK
		message(STATUS "Checking HDF5 ... OK!")
		
	else(HDF5_FOUND)
		message(FATAL_ERROR "Checking HDF5 ... error!!!")
		
	endif(HDF5_FOUND)
endif(WITH_HDF5)

target_link_libraries(imagemanager stitcher )
target_link_libraries(imagemanager crossmips )
