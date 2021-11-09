
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/jba/newmat11
QT += widgets
macx{
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
#    CONFIG += x86_64
}

win32{
    LIBS += -L$$V3DMAINPATH/common_lib/mingw -lv3dnewmat
}

unix:!macx {
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}

HEADERS	+= Image_enhancement_Anisotropy_plugin.h
SOURCES	+= Image_enhancement_Anisotropy_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(image_anisotropy_map)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/image_analysis/image_anisotropy_map/
