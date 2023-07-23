
TEMPLATE	= lib
QT += widgets
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include
#INCLUDEPATH     += Python/include/

#LIBS += -L"$$_PRO_FILE_PWD_/Python/libs/"

HEADERS	+= imPreProcess_plugin.h \
    bilateral_filter.h \
    fun_fft.h \
    helpFunc.h \
    morphology.h
SOURCES	+= imPreProcess_plugin.cpp \
    fun_fft.cpp \
    helpFunc.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(imPreProcess)
DESTDIR	= $$VAA3DPATH/bin/plugins/imPreProcess/

