
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include

INCLUDEPATH += main 

HEADERS       += channel_split_plugin.h

SOURCES       += channel_split_plugin.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage_create.cpp

TARGET        = $$qtLibraryTarget(channelsplit)
#DESTDIR       = ../../v3d/plugins/color_channel/split_extract_channels
DESTDIR       = $$V3DMAINPATH/../bin/plugins/color_channel/split_extract_channels

