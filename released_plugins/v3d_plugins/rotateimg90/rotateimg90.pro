# a demo program written by Hanchuan Peng
# 2009-08-14

TEMPLATE      = lib
CONFIG       += plugin warn_off 
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = rotateimg90.h
SOURCES       = rotateimg90.cpp
TARGET        = $$qtLibraryTarget(rotateimg90)   #this should be the project name, i.e. the name of the .pro file
DESTDIR       = ../../v3d/plugins/image_geometry/Rotate_Image #better set a subdirectory here so that the plugin will be arranged nicely 

