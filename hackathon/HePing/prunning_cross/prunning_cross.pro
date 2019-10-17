
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../vaa3d_tools
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/released_plugins/v3d_plugins/swc_to_maskimage
HEADERS	+= prunning_cross_plugin.h \
    prunning.h\
    $$VAA3DPATH/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h
SOURCES	+= prunning_cross_plugin.cpp \
    prunning.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.cpp

TARGET	= $$qtLibraryTarget(prunning_cross)
DESTDIR	= $$VAA3DPATH/bin/plugins/prunning_cross/

