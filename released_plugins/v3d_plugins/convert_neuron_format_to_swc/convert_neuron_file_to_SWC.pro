
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG += debug_and_release
#CONFIG	+= x86_64

V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= convert_neuron_file_to_SWC_plugin.h \
    asc_to_swc.h
SOURCES	+= convert_neuron_file_to_SWC_plugin.cpp \
    asc_to_swc.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
#SOURCES += main.cpp

#=============L-Measure===============
#HEADERS += Lm/Neuron.h \
#            Lm/ParameterList.h \
#            Lm/Random.h \
#            Lm/Segment.h \
#            Lm/Limit.h \
#            Lm/Func.h \
#            Lm/pca.h \
#            Lm/Parameter.h \
#            Lm/Vector.h \
#            Lm/RString.h \
#            Lm/Elaboration.h

#SOURCES += Lm/Neuron.cpp \
#            Lm/ParameterList.cpp \
#            Lm/Random.cpp \
#            Lm/Segment.cpp \
#            Lm/Limit.cpp \
#            Lm/Func.cpp \
#            Lm/pca.c \
#            Lm/Parameter.cpp \
#            Lm/Vector.cpp \
#            Lm/RString.cpp \
#            Lm/Elaboration.cpp

TARGET	= $$qtLibraryTarget(convert_Neurolucida_ASC_file_to_SWC)
DESTDIR	= $$V3DMAINPATH/bin/plugins/data_IO/convert_Neurolucida_ASC_file_to_SWC/
