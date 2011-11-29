# generate a linker file for SWC/APO/etc files under the given directory
# by Lei Qu
# 2009-12-30
# last change: by Hanchuan Peng, 2010-4-20

TEMPLATE      = lib
CONFIG       += qt plugin warn_off 

INCLUDEPATH  += ../../../v3d_main/basic_c_fun  

HEADERS       = linker_file_generator.h
SOURCES       = linker_file_generator.cpp
SOURCES       += ../../../v3d_main/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(linker_file_generator)   #this should be the project name, i.e. the name of the .pro file

DESTDIR       = ../../v3d/plugins/linker_file/Linker_File_Generator  

