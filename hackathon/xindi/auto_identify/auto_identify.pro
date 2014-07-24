
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= auto_identify_plugin.h
SOURCES	+= auto_identify_plugin.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(auto_identify)
DESTDIR	= ../../../../v3d_external/bin/plugins/auto_identify/
#DESTDIR = C:\Users\xindia\Downloads\vaa3d_bin_msvc_64bit_v2868\vaa3d_bin_msvc_64bit_v2868/plugins/auto_identify/

