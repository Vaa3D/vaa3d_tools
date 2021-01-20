
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
CONFIG += qaxcontainer
#QMAKE_CXXFLAGS += /FS
#CONFIG	+= x86_64
VAA3DPATH = D:/vaa3d_2013/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= Mean_Of_Region_plugin.h \
    extractMean_gui.h
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES	+= Mean_Of_Region_plugin.cpp \
    calculate_the_mean.cpp \
    extractMean_gui.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(Mean_Of_Region)
DESTDIR	= $$VAA3DPATH/bin/plugins/Mean_Of_Region/
