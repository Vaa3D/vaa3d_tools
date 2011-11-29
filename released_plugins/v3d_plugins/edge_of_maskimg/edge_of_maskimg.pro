# a demo program written by Hanchuan Peng
# 2009-05-30

TEMPLATE      = lib
CONFIG       += plugin warn_off 
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = edge_of_maskimg.h
SOURCES       = edge_of_maskimg.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(edge_of_maskimg)   #this should be the project name, i.e. the name of the .pro file
DESTDIR       = ../../v3d/plugins/image_edge_detection/Edge_Extraction_from_Mask_Image    #better set a subdirectory here so that the plugin will be arranged nicely 

