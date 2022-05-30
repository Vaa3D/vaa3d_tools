
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
QT += widgets


HEADERS	+= NeuronQuerySystem_plugin.h \
    basicinfo.h \
    neuronquerymainwindow.h
SOURCES	+= NeuronQuerySystem_plugin.cpp \
    basicinfo.cpp \
    neuronquerymainwindow.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(NeuronQuerySystem)
DESTDIR	= $$VAA3DPATH/bin/plugins/NeuronQuerySystem/

FORMS += \
    neuronquerymainwindow.ui
