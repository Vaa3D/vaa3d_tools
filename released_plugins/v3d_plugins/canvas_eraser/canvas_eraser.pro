# a demo program written by Hanchuan Peng
# 2009-08-14

TEMPLATE      = lib
CONFIG       += plugin warn_off 
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = canvas_eraser.h
SOURCES       = canvas_eraser.cpp
TARGET        = $$qtLibraryTarget(canvas_eraser)   #this should be the project name, i.e. the name of the .pro file
DESTDIR       = ../../v3d/plugins/pixel_intensity/Canvas_Eraser #better set a subdirectory here so that the plugin will be arranged nicely 

