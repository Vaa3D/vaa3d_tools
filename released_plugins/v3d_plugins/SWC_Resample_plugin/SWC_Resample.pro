
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= SWC_Resample_plugin.h
HEADERS += ../../../released_plugins/v3d_plugins/resample_swc/resampling.h
HEADERS += $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.h
HEADERS += $$V3DMAINPATH/v3d_main/neuron_editing/neuron_xforms.h

SOURCES	+= SWC_Resample_plugin.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/v3d_main/neuron_editing/neuron_xforms.cpp

TARGET	= $$qtLibraryTarget(SWC_Resample)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_utilities/swc_gui_resample/
