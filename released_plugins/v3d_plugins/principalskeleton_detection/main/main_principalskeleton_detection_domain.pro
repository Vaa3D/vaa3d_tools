#general principal skeleton detection
# by Lei Qu
# 2009-12-2

TEMPLATE = app
CONFIG += qt 
#QT -= gui # Only the core module is used

INCLUDEPATH += ../../../v3d_main/basic_c_fun/
INCLUDEPATH += ../../../v3d_main/jba/newmat11

LIBS += -ltiff
LIBS += -L../../../v3d_main/jba/c++ -lv3dnewmat

HEADERS += ../../../v3d_main/basic_c_fun/v3d_message.h
HEADERS += ../../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS += ../../../v3d_main/basic_c_fun/stackutil.h
HEADERS += ../../../v3d_main/basic_c_fun/mg_image_lib.h
HEADERS += ../../../v3d_main/basic_c_fun/mg_utilities.h
HEADERS += ../../../v3d_main/worm_straighten_c/spline_cubic.h
#HEADERS += ../../../v3d_main/q_skeletonbased_warp_sub2tar/q_neurontree_segmentation.h
HEADERS += q_morphology.h
HEADERS += q_principalskeleton_detection.h

SOURCES += ../../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += ../../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += ../../../v3d_main/basic_c_fun/stackutil.cpp
SOURCES += ../../../v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES += ../../../v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES += ../../../v3d_main/worm_straighten_c/spline_cubic.cpp
#SOURCES += ../../../v3d_main/q_skeletonbased_warp_sub2tar/q_neurontree_segmentation.cpp
SOURCES += q_morphology.cpp
SOURCES += q_principalskeleton_detection.cpp

SOURCES += main_principalskeleton_detection_domain.cpp


