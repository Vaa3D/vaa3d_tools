
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QT  += opengl
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/3drenderer


HEADERS	+= histogram_plugin.h
HEADERS	+= histogram_func.h
HEADERS	+= histogram_gui.h

SOURCES	= histogram_plugin.cpp
SOURCES	+= histogram_func.cpp
SOURCES	+= histogram_gui.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp


TARGET	= $$qtLibraryTarget(histogram)
DESTDIR	= $$VAA3DPATH/bin/plugins/histogram/
