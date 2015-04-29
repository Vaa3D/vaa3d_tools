
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= SWC_Resample_plugin.h
HEADERS += ../../../released_plugins/v3d_plugins/resample_swc/resampling.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.h
HEADERS += $$VAA3DPATH/v3d_main/neuron_editing/neuron_xforms.h

SOURCES	+= SWC_Resample_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/neuron_xforms.cpp

TARGET	= $$qtLibraryTarget(SWC_Resample)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/swc_gui_resample/
