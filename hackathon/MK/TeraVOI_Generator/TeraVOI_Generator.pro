
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun \
                ../../../v3d_main/common_lib/include

HEADERS	+= TeraVOI_Generator_plugin.h \
        ../../../released_plugins/v3d_plugins/gsdt/common_dialog.h \
        ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h \
    VOI_func.h

SOURCES	+= TeraVOI_Generator_plugin.cpp \
    voi_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp \
        VOI_func.cpp \
        ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp \
        ../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.cpp \

TARGET	= $$qtLibraryTarget(TeraVOI_Generator)
DESTDIR	= $$VAA3DPATH/bin/plugins/TeraVOI_Generator/
