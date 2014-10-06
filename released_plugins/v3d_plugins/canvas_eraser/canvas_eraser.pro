# a demo program written by Hanchuan Peng
# 2009-08-14
# updated by Jianlong Zhou 2012-04-18


TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += main 

HEADERS += canvas_eraser.h

SOURCES  = canvas_eraser.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(canvas_eraser)   #this should be the project name, i.e. the name of the .pro file
DESTDIR       = $$V3DMAINPATH/../bin/plugins/pixel_intensity/Canvas_Eraser #better set a subdirectory here so that the plugin will be arranged nicely 
