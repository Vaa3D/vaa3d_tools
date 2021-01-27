
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../..
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d


HEADERS	+= color_render_ESWC_features_plugin.h \
    color_render_eswc_dialog.h
SOURCES	+= color_render_ESWC_features_plugin.cpp \
    color_render_eswc_dialog.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(color_render_ESWC_features)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_utilities/color_render_ESWC_features/
