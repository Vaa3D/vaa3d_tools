include_directories(${TIFF_INCLUDE_DIR})

if(NOT Qt5Core_FOUND)
  QT4_WRAP_CPP(QT_INTERFACE_MOC_SRCS
    basic_thread.h
  )
else()
  QT5_WRAP_CPP(QT_INTERFACE_MOC_SRCS
      basic_thread.h
  )
endif()

set(V3DInterface_SRCS
  ${QT_INTERFACE_MOC_SRCS}
  basic_4dimage.cpp
  # basic_memory.cpp
  basic_surf_objs.cpp
  imageio_mylib.cpp
  mg_image_lib.cpp
  mg_utilities.cpp
  stackutil.cpp
  v3d_message.cpp
  # vcdiff.cpp
  ../neuron_annotator/utility/ImageLoaderBasic.cpp
  ../io/v3d_nrrd.cpp
  ../io/asc_to_swc.cpp  # @FIXED by Alessandro on 2015-05-06: added missing .cpp file
  basic_4dimage_create.cpp
  )

if (WIN32)
    set(V3DInterface_SRCS ${V3DInterface_SRCS} vcdiff.cpp)
else()
    set(V3DInterface_SRCS ${V3DInterface_SRCS} imageio_mylib.cpp)
endif()

add_library(V3DInterface ${V3DInterface_SRCS})

if (UNIX)
  ExternalProject_Add (
    LibBz2
    URL "${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/src_packages/bzip2-1.0.6.tar.gz"
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/bz2
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make     # Command to drive the native build
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
    )

  set(BZ2_LIB "${CMAKE_CURRENT_BINARY_DIR}/bz2/libbz2.a") # bZip2

  ExternalProject_Add (LibTeem
    URL "${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/src_packages/teem-1.11.0-src.tar.gz"
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/teem/src
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/teem/build
    UPDATE_COMMAND ""
    CMAKE_CACHE_ARGS
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_FLAGS:STRING=-fPIC
    -DCMAKE_C_FLAGS:STRING=-fPIC
    -DBUILD_TESTING:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=OFF
    ${CMAKE_PROJECT_INCLUDE_EXTERNAL_PROJECT_ARG}
    -DTeem_USE_LIB_INSTALL_SUBDIR:BOOL=ON
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=OFF
    -DTeem_PTHREAD:BOOL=OFF
    -DTeem_BZIP2:BOOL=OFF
    #-DTeem_ZLIB:BOOL=ON
    -DTeem_PNG:BOOL=OFF
    #-DZLIB_ROOT:PATH=${ZLIB_ROOT}
    #-DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}
    #-DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY}
    #-DPNG_PNG_INCLUDE_DIR:PATH=${PNG_INCLUDE_DIR}
    INSTALL_COMMAND ""
    )

  include_directories("${CMAKE_CURRENT_BINARY_DIR}/teem/build/include")
  set(TEEM_LIB "${CMAKE_CURRENT_BINARY_DIR}/teem/build/bin/libteem.a" )
endif(UNIX)

add_definitions(-DTEEM_STATIC) # otherwise Windows  will have link errors

if (UNIX)
  target_link_libraries(V3DInterface ${TEEM_LIB}  ${BZ2_LIB})
  add_dependencies(V3DInterface LibBz2)
  add_dependencies(V3DInterface LibTeem)
endif(UNIX)

if (MSVC)
  message("CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
  if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES ".*64")  # AMD64 is returned, not x86_64
      find_library( TEEM_LIB "teem" "${CMAKE_BINARY_DIR}/v3d_main/common_lib/winlib64")
      find_library( BZ2_LIB "libbz2" "${CMAKE_BINARY_DIR}/v3d_main/common_lib/winlib64")
   else()
       find_library( TEEM_LIB "teem" "${CMAKE_BINARY_DIR}/v3d_main/common_lib/winlib")
       find_library( BZ2_LIB "libbz2" "${CMAKE_BINARY_DIR}/v3d_main/common_lib/winlib")
   endif()
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../common_lib/include")
  target_link_libraries(V3DInterface ${TEEM_LIB} ${BZ2_LIB})
endif(MSVC)


target_link_libraries(V3DInterface ${TIFF_LIBRARY})
if(NOT Qt5Core_FOUND)
  target_link_libraries(V3DInterface ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
else()
  target_link_libraries(V3DInterface Qt5::Widgets Qt5::Core)
endif()

if (NOT WIN32)
    target_link_libraries(V3DInterface mylib_tiff)
endif()

if(V3D_USE_OWN_TIFF)
    add_dependencies(V3DInterface CustomTiffLibrary)
endif(V3D_USE_OWN_TIFF)

# 64 bit AMD Linux requires -fPIC flag for linking static libraries --CMB
if(UNIX AND NOT APPLE)
  if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    set_target_properties(V3DInterface PROPERTIES COMPILE_FLAGS -fPIC)
  endif()
endif()

