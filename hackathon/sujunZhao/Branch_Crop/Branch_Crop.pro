
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/braincenter8/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= Branch_Crop_plugin.h \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h \
    ../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h
SOURCES	+= Branch_Crop_plugin.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp \
    ../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(Branch_Crop)
DESTDIR	= $$VAA3DPATH/bin/plugins/Branch_Crop/
