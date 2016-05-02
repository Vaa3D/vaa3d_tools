# Create plugin
set(PLUGIN_HEADERS lobeseg_gui.h lobeseg_plugin.h) # Q_OBJECT macro containing files only
set(PLUGIN_DIRECTORY_NAME image_segmentation/lobeseger)
set(PLUGIN_SOURCES
    lobeseg_plugin.cpp
    lobeseg_func.cpp
    getopt.c
    lobeseg_main/lobeseg.cpp
    ${V3D_MAIN}/worm_straighten_c/bdb_minus.cpp
    ${V3D_MAIN}/worm_straighten_c/mst_prim_c.cpp
    ${V3D_MAIN}/worm_straighten_c/bfs_1root.cpp
)
include_directories("${TIFF_INCLUDE_DIR}")
configure_v3d_plugin(lobeseg)

# Create standalone executable
#if(NOT WIN32)
#add_executable(lobe_seger
#    ${V3D_MAIN}/worm_straighten_c/bdb_minus.cpp
#    lobeseg_main/lobeseg.cpp
#    lobeseg_main/main_backgnd_lobeseg_bl.cpp
#    ${V3D_MAIN}/basic_c_fun/mg_utilities.cpp
#    ${V3D_MAIN}/basic_c_fun/mg_image_lib.cpp
#    ${V3D_MAIN}/basic_c_fun/stackutil.cpp
#    ${V3D_MAIN}/worm_straighten_c/mst_prim_c.cpp
#    ${V3D_MAIN}/worm_straighten_c/bfs_1root.cpp
#)
#target_link_libraries(lobe_seger ${TIFF_LIBRARY})
#set_target_properties(lobe_seger PROPERTIES
#    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#install(TARGETS lobe_seger
#    DESTINATION bin
#)
#endif(NOT WIN32)
