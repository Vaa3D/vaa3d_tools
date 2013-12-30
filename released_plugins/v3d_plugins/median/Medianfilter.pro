
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/jba/newmat11

macx{
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
#    CONFIG += x86_64
}

win32{
    LIBS += -L$$V3DMAINPATH/common_lib/winlib64 -llibnewmat
}

unix:!macx {
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}

INCLUDEPATH += main 


HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h

HEADERS	+= Medianfilter_plugin.h
SOURCES	+= Medianfilter_plugin.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp

TARGET	= $$qtLibraryTarget(medianfilter)
#DESTDIR	= ../../v3d/plugins/image_filters/median_filter/
DESTDIR	= $$V3DMAINPATH/../bin/plugins/image_filters/median_filter/
