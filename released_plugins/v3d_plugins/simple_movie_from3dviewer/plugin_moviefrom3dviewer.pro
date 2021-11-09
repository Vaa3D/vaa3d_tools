#created by Lei Qu
# Last change: by Hanchuan Peng, 2010-11-23

TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64
V3DMAINPATH =../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
QT += widgets

HEADERS       = plugin_moviefrom3dviewer.h
SOURCES       = plugin_moviefrom3dviewer.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(moviefrom3dviewer)

DESTDIR       =  $$V3DMAINPATH/bin/plugins/movies_and_snapshots/Simple_Movie_Maker

