
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/common_lib/include

HEADERS	+= mipZSlices_plugin.h
SOURCES	+= mipZSlices_plugin.cpp

HEADERS += $$VAA3DPATH/basic_c_fun/basic_memory.h

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_4dimage_create.cpp

SOURCES += $$VAA3DPATH/basic_c_fun/basic_memory.cpp

TARGET	= $$qtLibraryTarget(mipZSlices)
DESTDIR       =  $$VAA3DPATH/../bin/plugins/image_projection/maximum_intensity_projection_Z_Slices/
