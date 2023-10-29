
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/penglab/GitRepo/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= ImageLib_plugin.h \
    processlib.h
SOURCES	+= ImageLib_plugin.cpp \
    processlib.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(ImageLib)
DESTDIR	= $$VAA3DPATH/bin/plugins/ImageLib/
