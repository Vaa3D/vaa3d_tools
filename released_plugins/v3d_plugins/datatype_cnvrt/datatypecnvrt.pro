
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun

HEADERS       = datatypecnvrt.h 

SOURCES       = datatypecnvrt.cpp

TARGET        = $$qtLibraryTarget(datatypeconvert)
DESTDIR       = ../../v3d/plugins/data_type/Convert_8_16_32_bits_data

