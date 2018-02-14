# set up our directory structure for output libraries and binaries
if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  if(UNIX)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
  else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  endif()
endif()

# set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

# disable annoying warnings on MSVC compilers
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	warnings_disable(CMAKE_CXX_FLAGS /wd4290) # vs2012: can't properly handle exceptions
	warnings_disable(CMAKE_CXX_FLAGS /wd4996) # vs2012: complains about unsafe standard C++ functions
endif()

#set up APPLE
if(APPLE)
	# bundle
	set(MACOSX_BUNDLE_NAME "TeraStitcher-Bundle-Name")
	#set(prefix "${MACOSX_BUNDLE_NAME}.app/Contents")
	#set(INSTALL_INCLUDE_DIR "${prefix}/${INSTALL_INCLUDE_DIR}")
	#set(INSTALL_RUNTIME_DIR "${prefix}/MacOS")
	#set(INSTALL_LIBRARY_DIR "${prefix}/${INSTALL_LIBRARY_DIR}")
	#set(INSTALL_ARCHIVE_DIR "${prefix}/${INSTALL_ARCHIVE_DIR}")
	#set(INSTALL_DATA_DIR    "${prefix}/${INSTALL_DATA_DIR}")
	#set(INSTALL_DOC_DIR     "${prefix}/${INSTALL_DOC_DIR}")
	#set(INSTALL_CMAKE_DIR   "${prefix}/Resources")
  
	# backward compatibility for MacOS X 10.6+
	
	#SET(MAC_COVERAGE_COMPILE_FLAGS "-mmacosx-version-min=10.6")
	#SET(MAC_COVERAGE_LINK_FLAGS    "-mmacosx-version-min=10.6")
	#if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${MAC_COVERAGE_COMPILE_FLAGS}" )
		#SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${MAC_COVERAGE_LINK_FLAGS}" )
	#endif()
endif()