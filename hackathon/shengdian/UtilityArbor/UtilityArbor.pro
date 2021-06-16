
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= UtilityArbor_plugin.h \
    Utility_function.h \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES	+= UtilityArbor_plugin.cpp \
    Utility_function.cpp \
    ../../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(UtilityArbor)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/UtilityArbor/
