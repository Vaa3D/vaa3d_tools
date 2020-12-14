
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= MorphoHub_plugin.h \
    GUI/morphohub_gui.h \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h \
    basic_conf.h \
    Dialogs/imagedatamanagement.h \
    Service/screenwall.h \
    Service/screenwall_func.h \
    Dialogs/usermanagement.h
SOURCES	+= MorphoHub_plugin.cpp \
    GUI/morphohub_gui.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp \
    basic_conf.cpp \
    Dialogs/imagedatamanagement.cpp \
    Service/screenwall.cpp \
    Dialogs/usermanagement.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(MorphoHub)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/MorphoHub/
