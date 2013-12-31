# a demo program written by Hanchuan Peng
# 2009-08-14
# updated by Jianlong Zhou 2012-04-16

TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include

#HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h

HEADERS += rotateimg90.h
SOURCES  = rotateimg90.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
#SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp

TARGET        = $$qtLibraryTarget(rotateimg90)   #this should be the project name, i.e. the name of the .pro file
DESTDIR       = ../../v3d/plugins/image_geometry/Rotate_Image #better set a subdirectory here so that the plugin will be arranged nicely 
