
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/jba/newmat11
macx{
    LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dnewmat
#    CONFIG += x86_64
}

win32{
    LIBS += -L$$V3DMAINPATH/common_lib/mingw -lv3dnewmat
}

unix:!macx {
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}
QT += widgets
HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h \
    common_value.h
INCLUDEPATH += main 

HEADERS += main/heap.h
HEADERS += main/fastmarching_dt.h

HEADERS	= gsdt_plugin.h
HEADERS	+= gsdt_func.h

SOURCES	= gsdt_plugin.cpp
SOURCES	+= gsdt_func.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp

HEADERS += common_dialog.h

TARGET	= $$qtLibraryTarget(gsdt)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/image_filters/Grayscale_Image_Distance_Transform/

