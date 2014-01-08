# affine transfer the image based on the given point sets
# by Lei Qu
# 2010-03-07

TEMPLATE = app
CONFIG += qt
CONFIG += x86_64
#QT -= gui # Only the core module is used

V3DMAINPATH = ../../../v3d_external/v3d_main

INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/jba/newmat11
macx{
    LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
#    CONFIG += x86_64
}

win32{
}

unix:!macx {
    #LIBS += -L$$V3DMAINPATH/common_lib/lib -lv3dtiff
    LIBS += -L$$V3DMAINPATH/common_lib/lib -ltiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}

HEADERS += $$V3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS += $$V3DMAINPATH/basic_c_fun/stackutil.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_image_lib.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_utilities.h
HEADERS += $$V3DMAINPATH/jba/c++/convert_type2uint8.h

SOURCES += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/jba/c++/convert_type2uint8.cpp
SOURCES += main_affinetransform.cpp



