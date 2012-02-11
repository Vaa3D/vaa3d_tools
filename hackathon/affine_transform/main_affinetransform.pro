# affine transfer the image based on the given point sets
# by Lei Qu
# 2010-03-07

TEMPLATE = app
CONFIG += qt 
CONFIG += x86_64
#QT -= gui # Only the core module is used

INCLUDEPATH += ../../v3d_main/jba/newmat11
LIBS += -L../../v3d_main/jba/c++ -lv3dnewmat
LIBS += -ltiff

HEADERS += ../../v3d_main/basic_c_fun/v3d_message.h
HEADERS += ../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS += ../../v3d_main/basic_c_fun/stackutil.h
HEADERS += ../../v3d_main/basic_c_fun/mg_image_lib.h
HEADERS += ../../v3d_main/basic_c_fun/mg_utilities.h
HEADERS += ../../v3d_main_internal/jba/c++/convert_type2uint8.h

SOURCES += ../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += ../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += ../../v3d_main/basic_c_fun/stackutil.cpp
SOURCES += ../../v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES += ../../v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES += ../../v3d_main_internal/jba/c++/convert_type2uint8.cpp
SOURCES += main_affinetransform.cpp



