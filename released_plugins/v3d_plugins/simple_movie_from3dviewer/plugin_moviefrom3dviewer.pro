#created by Lei Qu
# Last change: by Hanchuan Peng, 2010-11-23

TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64

INCLUDEPATH  += ../../../v3d_main/basic_c_fun 

HEADERS       = plugin_moviefrom3dviewer.h
SOURCES       = plugin_moviefrom3dviewer.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(moviefrom3dviewer)

DESTDIR       = ../../v3d/plugins/movies/Simple_Movie_Maker
#DESTDIR       = ../../../v3d_main/v3d/plugins/Simple_Movie_Maker

