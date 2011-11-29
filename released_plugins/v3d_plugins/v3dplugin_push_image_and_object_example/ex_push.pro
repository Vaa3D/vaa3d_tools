
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64
INCLUDEPATH  += ../../../v3d_main/basic_c_fun 
HEADERS       = ex_push.h
SOURCES       = ex_push.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(ex_push)
DESTDIR       = ../../v3d/plugins/Vaa3D_PluginInterface_Demos/3D_viewer_data_push_and_display

