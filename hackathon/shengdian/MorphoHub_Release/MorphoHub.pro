
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= MorphoHub_plugin.h \
    GUI/morphohub_gui.h \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h \
    Service/screenwall.h \
    Service/screenwall_func.h \
    morphohub_dbms_basic.h \
    GUI/newimage_gui.h \
    Generator/bouton_fun.h
SOURCES	+= MorphoHub_plugin.cpp \
    GUI/morphohub_gui.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp \
    Service/screenwall.cpp \
    morphohub_dbms_basic.cpp \
    GUI/newimage_gui.cpp \
    Generator/bouton_fun.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(MorphoHub)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/MorphoHub/
