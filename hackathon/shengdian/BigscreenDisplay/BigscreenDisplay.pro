
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= BigscreenDisplay_plugin.h \
    BigScreenDispaly_func.h \
    BigscreenControlDialog.h \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.h

SOURCES	+= BigscreenDisplay_plugin.cpp \
    BigScreenDisplay_func.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(BigscreenDisplay)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/BigscreenDisplay/
