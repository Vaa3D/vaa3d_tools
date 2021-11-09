TEMPLATE      = lib
CONFIG       += qt plugin warn_off
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

QT += widgets
HEADERS       = roi_editor.h
SOURCES       = roi_editor.cpp
SOURCES      += $$VAA3DPATH/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(roi_editor)
DESTDIR       = $$VAA3DPATH/bin/plugins/image_ROI/ROI_Editor
