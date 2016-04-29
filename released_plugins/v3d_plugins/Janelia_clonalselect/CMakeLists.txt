set(PLUGIN_HEADERS clonalselect.h clonalselect_gui.h) # Q_OBJECT macro containing files only
set(PLUGIN_DIRECTORY_NAME FlyWorkstation_utilities/clonalSelect)
set(PLUGIN_SOURCES clonalselect_gui.cpp clonalselect_core.cpp clonalselect.cpp clonalselect.h)
include_directories("${TIFF_INCLUDE_DIR}")
configure_v3d_plugin(clonalselect)
target_link_libraries(clonalselect ${TIFF_LIBRARY})
