
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
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

INCLUDEPATH += main 


HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_utilities.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_image_lib.h
HEADERS += $$V3DMAINPATH/basic_c_fun/stackutil.h

HEADERS	+= 2D_Enhancement_plugin.h
SOURCES	+= 2D_Enhancement_plugin.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp

TARGET	= $$qtLibraryTarget(2D_Enhancement)
DESTDIR	= ../../../../v3d_external/bin/plugins/Adaptive_Thresholding/2D_Enhancement

