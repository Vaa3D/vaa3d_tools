
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
HEADERS       = ex_call.h
SOURCES       = ex_call.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(ex_call)
DESTDIR       = $$V3DMAINPATH/../bin/plugins/Vaa3D_PluginInterface_Demos/call_each_other
QT += widgets
