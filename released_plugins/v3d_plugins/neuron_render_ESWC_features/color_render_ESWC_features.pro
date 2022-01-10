
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
QT += widgets
HEADERS	+= color_render_ESWC_features_plugin.h \
    color_render_eswc_dialog.h
SOURCES	+= color_render_ESWC_features_plugin.cpp \
    color_render_eswc_dialog.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(color_render_ESWC_features)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_utilities/color_render_ESWC_features/
