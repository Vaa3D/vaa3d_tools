
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= tip_marker_func.h \
           Tip_marker_finder_plugin.h
HEADERS += ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h
HEADERS += ../../../released_plugins/v3d_plugins/gsdt/common_dialog.h

SOURCES	+= Tip_marker_finder_plugin.cpp \
           tip_marker_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.cpp

TARGET	= $$qtLibraryTarget(Tip_marker_finder)
DESTDIR	= $$VAA3DPATH/bin/plugins/Tip_marker_finder/


FORMS += TipMarkerFinderUI.ui
