# affine+TPS transfer the swc based on the given point sets
# by Lei Qu
# 2010-03-18

TEMPLATE = app
CONFIG += qt 
#QT -= gui # Only the core module is used

INCLUDEPATH += ../../v3d_main/jba/newmat11
LIBS += -L../../v3d_main/jba/c++ -lv3dnewmat

HEADERS += ../../v3d_main/basic_c_fun/v3d_message.h
HEADERS += ../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS += q_warp_affine_tps.h

SOURCES += ../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += ../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += q_warp_affine_tps.cpp
SOURCES += main_swcwarp_affine_tps.cpp


