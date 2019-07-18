
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH     =  ../../../../v3d_external/v3d_main
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include

HEADERS	+= interactive_segmentation_plugin.h
HEADERS	+= RandomWalk.h
HEADERS	+= NeuronSegmentation.h
HEADERS	+= NeuronEnhancementFilter.h
HEADERS	+= head.h
HEADERS	+= basic_4dimage.h
SOURCES	+= interactive_segmentation_plugin.cpp
SOURCES	+= RandomWalk.cpp
SOURCES	+= NeuronSegmentation.cpp
SOURCES	+= NeuronEnhancementFilter.cpp
SOURCES	+= enhance.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(interactive_segmentation)
DESTDIR = ../../../../v3d_external/bin/plugins/interactive_segmentation/
