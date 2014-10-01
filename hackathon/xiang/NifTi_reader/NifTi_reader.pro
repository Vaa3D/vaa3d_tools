
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= ./include
LIBS += ./lib/nifticdf.lib
LIBS += ./lib/niftiio.lib
LIBS += ./lib/znz.lib

HEADERS	+= NifTi_reader_plugin.h
SOURCES	+= NifTi_reader_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(NifTi_reader)
DESTDIR	= ./release