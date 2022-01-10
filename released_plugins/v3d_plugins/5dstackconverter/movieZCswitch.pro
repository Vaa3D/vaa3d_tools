
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
V3DMAINPATH =../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH  +=  $$V3DMAINPATH/common_lib/include
QT += widgets
HEADERS       = movieZCswitch.h
SOURCES       = movieZCswitch.cpp
SOURCES       += $$V3DMAINPATH//basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(movieZCswitch)
DESTDIR       = $$V3DMAINPATH/../bin/plugins/data_type/5D_Stack_Converter



