TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64

VAA3DPATH = ../../../../v3d_external
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include

HEADERS       = snapshots_3dviewer.h
SOURCES       = snapshots_3dviewer.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
HEADERS      += $$V3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
SOURCES      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
TARGET        = $$qtLibraryTarget(snapshots_3dviewer)

DESTDIR       = $$VAA3DPATH/bin/plugins/snapshots_3dviewer

