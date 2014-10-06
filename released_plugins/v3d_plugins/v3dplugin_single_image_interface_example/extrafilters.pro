
TEMPLATE      = lib
CONFIG       += plugin warning_off 
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = extrafiltersplugin.h
SOURCES       = extrafiltersplugin.cpp
TARGET        = $$qtLibraryTarget(extrafilters)
DESTDIR       = ../../../../v3d_external/bin/plugins/Vaa3D_PluginInterface_Demos/Single_Image_Interface #win32 qmake couldn't handle space in path

