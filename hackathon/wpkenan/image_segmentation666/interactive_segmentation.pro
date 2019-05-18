
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH     =  ../../../../v3d_external/v3d_main
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include
INCLUDEPATH     += $$V3DMAINPATH/../../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_simpletracing


HEADERS	+= interactive_segmentation_plugin.h
HEADERS	+= HessEnhancement.h
HEADERS	+= NeuronSegmentation.h
SOURCES	+= interactive_segmentation_plugin.cpp
SOURCES	+= HessEnhancement.cpp
SOURCES	+= NeuronSegmentation.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(interactive_segmentation)
DESTDIR = ../../../../v3d_external/bin/plugins/wpkenanPlugin/interactive_segmentation/
