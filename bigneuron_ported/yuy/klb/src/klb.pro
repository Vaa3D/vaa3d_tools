
TEMPLATE = lib
CONFIG += qt plugin warn_off
#CONFIG	+= x86_64

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS   += -std=c++11

VAA3DPATH = ../../../..
V3DMAINPATH = $$VAA3DPATH/v3d_main
KLBDEP=./external

INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$KLBDEP/bzip2
INCLUDEPATH += $$KLBDEP/zlib

HEADERS += $$V3DMAINPATH/basic_c_fun/mg_utilities.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_image_lib.h
unix:HEADERS += $$V3DMAINPATH/basic_c_fun/imageio_mylib.h
HEADERS	+= klb_plugin.h

HEADERS	+= common.h
HEADERS	+= klb_ROI.h
HEADERS	+= klb_imageHeader.h
HEADERS	+= klb_Cwrapper.h
HEADERS	+= klb_circularDequeue.h
HEADERS	+= klb_imageIO.h

SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
unix:SOURCES += $$V3DMAINPATH/basic_c_fun/imageio_mylib.cpp
SOURCES	+= klb_plugin.cpp

SOURCES	+= klb_Cwrapper.cpp
SOURCES	+= klb_circularDequeue.cpp
SOURCES	+= klb_imageIO.cpp
SOURCES	+= klb_ROI.cpp
SOURCES	+= klb_imageHeader.cpp

unix {
    LIBS += -L$$V3DMAINPATH/common_lib/lib -lv3dtiff
    LIBS += -L$$V3DMAINPATH/common_lib/src_packages/mylib_tiff -lmylib
    LIBS += -L$$KLBDEP/zlib -lz
    LIBS += -L$$KLBDEP/bzip2 -lbz2lib
}

TARGET	= $$qtLibraryTarget(klb_file_io)
DESTDIR	= $$VAA3DPATH/bin/plugins/data_IO/klb_file_io

