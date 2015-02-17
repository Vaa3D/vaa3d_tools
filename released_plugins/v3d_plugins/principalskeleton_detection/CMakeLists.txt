
set(PLUGIN_HEADERS plugin_principalskeleton_detection.h )
set(PLUGIN_SOURCES 
    ${V3D_BASIC_C_FUN_SOURCE_DIR}/v3d_message.cpp
    ${V3D_BASIC_C_FUN_SOURCE_DIR}/stackutil.cpp
    ${V3D_BASIC_C_FUN_SOURCE_DIR}/mg_image_lib.cpp
    ${V3D_BASIC_C_FUN_SOURCE_DIR}/mg_utilities.cpp
    ${V3D_BASIC_C_FUN_SOURCE_DIR}/basic_surf_objs.cpp
    ${V3D_BASIC_C_FUN_SOURCE_DIR}/../worm_straighten_c/spline_cubic.cpp
    q_morphology.cpp
    q_principalskeleton_detection.cpp
    q_neurontree_segmentation.cpp
    q_skeletonbased_warp_sub2tar.cpp
    plugin_principalskeleton_detection.cpp
)
set(PLUGIN_DIRECTORY_NAME principal_skeleton_detection)

include_directories(${TIFF_INCLUDE_DIR})
include_directories(${V3DMAINDIR}/jba/newmat11)
configure_v3d_plugin(principalskeleton_detection)
target_link_libraries(principalskeleton_detection ${TIFF_LIBRARY})
find_library(newmat_LIBRARY newmat11 PATH ${V3D_PLUGIN_LIBRARIES_PATH} REQUIRED)
target_link_libraries(principalskeleton_detection ${newmat_LIBRARY})


# TEMPLATE      = lib
# CONFIG       += plugin 
# 
# V3DMAINDIR = ../../../v3d_main
# 
# INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
# INCLUDEPATH  += $$V3DMAINDIR/jba/newmat11
# INCLUDEPATH  += $$V3DMAINDIR/common_lib/include
# 
# LIBS 	     += -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
# LIBS         += -L$$V3DMAINDIR/jba/c++ -lv3dnewmat
# 
# HEADERS      += $$V3DMAINDIR/basic_c_fun/v3d_message.h
# HEADERS	     += $$V3DMAINDIR/basic_c_fun/stackutil.h
# HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
# HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_utilities.h
# HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.h
# HEADERS      += $$V3DMAINDIR/worm_straighten_c/spline_cubic.h
# HEADERS      += q_morphology.h
# HEADERS      += q_principalskeleton_detection.h
# HEADERS      += q_neurontree_segmentation.h
# HEADERS      += q_skeletonbased_warp_sub2tar.h
# HEADERS      += plugin_principalskeleton_detection.h
# 
# SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
# SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
# SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp
# SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
# SOURCES      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.cpp
# SOURCES      += $$V3DMAINDIR/worm_straighten_c/spline_cubic.cpp
# SOURCES      += q_morphology.cpp
# SOURCES      += q_principalskeleton_detection.cpp
# SOURCES      += q_neurontree_segmentation.cpp
# SOURCES      += q_skeletonbased_warp_sub2tar.cpp
# SOURCES      += plugin_principalskeleton_detection.cpp
# 
# TARGET        = $$qtLibraryTarget(principalskeleton_detection)
# 
# DESTDIR       = ../../v3d/plugins/principal_skeleton_detection #win32 qmake couldn't handle space in path
# #DESTDIR       = ../../../v3d_main/v3d/plugins/principal_skeleton_detection #win32 qmake couldn't handle space in path
