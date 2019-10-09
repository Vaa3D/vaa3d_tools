
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/vs2013project/vaa3d_tools
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= cell_segmentation_plugin.h
SOURCES	+= cell_segmentation_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(cell_segmentation)
DESTDIR	= $$VAA3DPATH/bin/plugins/cell_segmentation/
