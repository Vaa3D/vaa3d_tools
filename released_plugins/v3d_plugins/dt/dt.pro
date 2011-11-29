
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = dt.h
SOURCES       = dt.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(dt)
DESTDIR       = ../../v3d/plugins/image_filters/Fast_Distance_Transform

