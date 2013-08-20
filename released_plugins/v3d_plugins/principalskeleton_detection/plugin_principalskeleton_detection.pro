
TEMPLATE      = lib
CONFIG       += plugin 

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/jba/newmat11
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

LIBS 	     += -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
LIBS         += -L$$V3DMAINDIR/jba/c++ -lv3dnewmat

HEADERS      += $$V3DMAINDIR/basic_c_fun/v3d_message.h
HEADERS	     += $$V3DMAINDIR/basic_c_fun/stackutil.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.h
HEADERS      += $$V3DMAINDIR/worm_straighten_c/spline_cubic.h
HEADERS      += q_morphology.h
HEADERS      += q_principalskeleton_detection.h
HEADERS      += q_neurontree_segmentation.h
HEADERS      += q_skeletonbased_warp_sub2tar.h
HEADERS      += plugin_principalskeleton_detection.h
#HEADERS      += main_principalskeleton_detection_domain_dofunc.h

SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINDIR/worm_straighten_c/spline_cubic.cpp
SOURCES      += q_morphology.cpp
SOURCES      += q_principalskeleton_detection.cpp
SOURCES      += q_neurontree_segmentation.cpp
SOURCES      += q_skeletonbased_warp_sub2tar.cpp
SOURCES      += plugin_principalskeleton_detection.cpp
#SOURCES      += main_principalskeleton_detection_domain_dofunc.cpp


TARGET        = $$qtLibraryTarget(principalskeleton_detection)

DESTDIR       = ../../v3d/plugins/principal_skeleton_detection #for vaa3d batch build
#DESTDIR       = ../../../bin/plugins/principal_skeleton_detection #for local test
