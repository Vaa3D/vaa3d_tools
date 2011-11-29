TEMPLATE      = lib
CONFIG       += qt plugin warn_off
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
HEADERS       = roi_editor.h
SOURCES       = roi_editor.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
TARGET        = $$qtLibraryTarget(roi_editor)
DESTDIR       = ../../v3d/plugins/image_ROI/ROI_Editor
