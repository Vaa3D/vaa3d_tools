TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64

V3DMAINPATH = ../../../../v3d_external
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/v3d

HEADERS       = snapshots_3dviewer.h
SOURCES       = snapshots_3dviewer.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
HEADERS      += $$V3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
SOURCES      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
HEADERS      += $$V3DMAINPATH/neuron_editing/neuron_xforms.h
SOURCES      += $$V3DMAINPATH/neuron_editing/neuron_xforms.cpp

TARGET        = $$qtLibraryTarget(snapshots_3dviewer)

DESTDIR       = $$V3DMAINPATH/bin/plugins/movies_and_snapshots/snapshots_3dviewer

