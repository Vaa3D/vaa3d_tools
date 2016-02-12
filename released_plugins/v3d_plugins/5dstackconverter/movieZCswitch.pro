
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
INCLUDEPATH  += ../../../../v3d_external/v3d_main/basic_c_fun
INCLUDEPATH  += ../../../../v3d_external/v3d_main/common_lib/include

HEADERS       = movieZCswitch.h
SOURCES       = movieZCswitch.cpp
SOURCES       +=  ../../../../v3d_external/v3d_main/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(movieZCswitch)
DESTDIR       = ../../../../v3d_external/bin/plugins/data_type/5D_Stack_Converter



