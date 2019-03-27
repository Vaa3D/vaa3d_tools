
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include
INCLUDEPATH	+= $$VAA3DPATH/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/worm_straighten_c
INCLUDEPATH     += $$VAA3DPATH/cellseg

INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2


HEADERS	+= refine_swc_plugin.h
HEADERS	+= refine_swc_funcs.h
HEADERS += Splines.h

SOURCES	+= refine_swc_plugin.cpp
SOURCES	+= refine_swc_funcs.cpp
SOURCES += Splines.cpp

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/neuron_editing/neuron_format_converter.cpp
SOURCES	+= $$VAA3DPATH/neuron_editing/v_neuronswc.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/tip_detection.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/autoseed.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/calculate_cover_scores.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.cpp \



#mask generation
HEADERS	+= ../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h
SOURCES	+= ../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.cpp

TARGET	= $$qtLibraryTarget(refine_swc)
#DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_utilities/refine_swc/
DESTDIR	= /home/penglab/PBserver/tmp/ding/vaa3d/plugins/neuron_utilities/refine_swc

QMAKE_CXXFLAGS +=-std=c++0x
