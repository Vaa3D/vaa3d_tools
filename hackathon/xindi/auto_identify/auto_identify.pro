
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= auto_identify_plugin.h
SOURCES	+= auto_identify_plugin.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(test)
win32 {
DESTDIR	= ../../../../vaa3d_win7_32bit_v2.707/plugins/auto_identify/
}
mac {
DESTDIR = $$VAA3DPATH/bin/plugins/auto_identify/
}
