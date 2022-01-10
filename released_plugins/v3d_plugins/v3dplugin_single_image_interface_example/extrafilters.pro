
TEMPLATE      = lib
CONFIG       += plugin warning_off 
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
HEADERS       = extrafiltersplugin.h
SOURCES       = extrafiltersplugin.cpp
TARGET        = $$qtLibraryTarget(extrafilters)
DESTDIR       = $$VAA3DPATH/../bin/plugins/Vaa3D_PluginInterface_Demos/Single_Image_Interface #win32 qmake couldn't handle space in path
QT += widgets
