# affine+TPS transfer the image based on the given point sets
# by Lei Qu
# 2010-10-27

TEMPLATE = app
CONFIG += qt 
#QT -= gui # Only the core module is used

INCLUDEPATH += ../../v3d_main/jba/newmat11

LIBS += -ltiff
LIBS += -L../../v3d_main/jba/c++ -lv3dnewmat

HEADERS += ../../v3d_main/basic_c_fun/v3d_message.h
HEADERS += ../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS += ../../v3d_main/basic_c_fun/stackutil.h
HEADERS += ../../v3d_main/basic_c_fun/mg_image_lib.h
HEADERS += ../../v3d_main/basic_c_fun/mg_utilities.h
HEADERS += q_warp_affine_tps.h

SOURCES += ../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += ../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += ../../v3d_main/basic_c_fun/stackutil.cpp
SOURCES += ../../v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES += ../../v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES += q_warp_affine_tps.cpp
SOURCES += main_imgwarp_affine_tps.cpp


