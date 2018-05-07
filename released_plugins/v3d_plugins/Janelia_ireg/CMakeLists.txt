set(PLUGIN_HEADERS ireg.h) # Q_OBJECT macro containing files only
set(PLUGIN_DIRECTORY_NAME image_registration/ireg)
set(PLUGIN_SOURCES
    ireg.cpp
    y_NiftiImageIO.cpp
    niftilib/nifti1_io.c
    niftilib/znzlib.c
)
include_directories(
    "${TIFF_INCLUDE_DIR}"
)
configure_v3d_plugin(ireg)
find_library(ZLIB z)
target_link_libraries(ireg mylib_tiff ${ZLIB})
