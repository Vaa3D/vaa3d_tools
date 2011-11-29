
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64
INCLUDEPATH  += ../../basic_c_fun
HEADERS       = imfill.h
SOURCES       = imfill.cpp
SOURCES      += ../../basic_c_fun/vcdiff.cpp
TARGET        = $$qtLibraryTarget(imfill)
DESTDIR       = ../../v3d/plugins/unfinished/imfill

