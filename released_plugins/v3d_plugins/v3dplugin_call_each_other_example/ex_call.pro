
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64
INCLUDEPATH  += ../../../v3d_main/basic_c_fun 
HEADERS       = ex_call.h
SOURCES       = ex_call.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(ex_call)
DESTDIR       = ../../v3d/plugins/Vaa3D_PluginInterface_Demos/call_each_other

