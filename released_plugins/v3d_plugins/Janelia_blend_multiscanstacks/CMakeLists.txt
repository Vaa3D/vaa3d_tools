set(PLUGIN_HEADERS imageblend.h) # Q_OBJECT macro containing files only
set(PLUGIN_DIRECTORY_NAME image_blending/blend_multiscanstacks)
set(PLUGIN_SOURCES imageblend.cpp basic_4dimage_ori.cpp imageblend.h)
include_directories("${TIFF_INCLUDE_DIR}")
include_directories("${Boost_INCLUDE_DIR}")
configure_v3d_plugin(blend_multiscanstacks)
if(WIN32 OR MINGW)
    target_link_libraries(blend_multiscanstacks ${FFTW_LIBRARY} ${TIFF_LIBRARY})
else(WIN32)
	find_library(MYLIB_TIFF_LIBRARY
    NAMES libmylib_tiff.a
    PATHS "${V3D_PLUGIN_LIBRARIES_PATH}")
    
	find_library(MY_TIFF_LIBRARY
    NAMES libmy_tiff.a
    PATHS "${V3D_PLUGIN_LIBRARIES_PATH}")
    
    target_link_libraries(blend_multiscanstacks ${FFTW_LIBRARY} mylib_tiff my_tiff ${TIFF_LIBRARY})
endif(WIN32 OR MINGW)
