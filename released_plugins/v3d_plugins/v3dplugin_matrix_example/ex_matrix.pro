
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = ex_matrix.h
SOURCES       = ex_matrix.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(ex_matrix)
DESTDIR       = ../../../../v3d_external/bin/plugins/Vaa3D_PluginInterface_Demos/call_each_other

