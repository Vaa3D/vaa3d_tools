
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= \
    openSWCDialog.h \
    get_missing_branches.h \
    missing_branch_detection_plugin.h
SOURCES	+= \
    openSWCDialog.cpp \
    get_missing_branches.cpp \
    missing_branch_detection_plugin.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

#HEADERS	+= $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h
#SOURCES	+= $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.cpp

TARGET	= $$qtLibraryTarget(missing_branch_detection)
DESTDIR	= $$VAA3DPATH/bin/plugins/missing_branch_detection/
