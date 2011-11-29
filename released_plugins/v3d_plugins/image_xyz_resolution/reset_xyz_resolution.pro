# a demo program written by Hanchuan Peng
# 2010-06-26

TEMPLATE      = lib
CONFIG       += plugin warn_off 
INCLUDEPATH  += ../../../v3d_main/basic_c_fun 
HEADERS       = reset_xyz_resolution.h
SOURCES       = reset_xyz_resolution.cpp
TARGET        = $$qtLibraryTarget(example_reset_xyz_resolution)   #this should be the project name, i.e. the name of the .pro file
DESTDIR       = ../../v3d/plugins/image_resolution/XYZ_Resolution #better set a subdirectory here so that the plugin will be arranged nicely 

