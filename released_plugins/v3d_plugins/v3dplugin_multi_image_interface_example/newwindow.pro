
TEMPLATE      = lib
CONFIG       += plugin warning_off
#CONFIG	     += X86_64 
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
INCLUDEPATH  += ../../../v3d_main/common_lib/include
INCLUDEPATH  += ../../../v3d_main/v3d
HEADERS       = newwindowplugin.h
SOURCES       = newwindowplugin.cpp
TARGET        = $$qtLibraryTarget(newwindow)
DESTDIR       = ../../../../v3d_external/bin/plugins/Vaa3D_PluginInterface_Demos/Multi_Image_Interface #win32 qmake couldn't handle space in path

