TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64

INCLUDEPATH  += ../../../v3d_main/basic_c_fun 

HEADERS       = snapshots_3dviewer.h
SOURCES       = snapshots_3dviewer.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(snapshots_3dviewer)

DESTDIR       = ../../../../v3d_external/bin/plugins/snapshots_3dviewer

