TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
CONFIG  -= bundle_off
V3DMAINPATH  =  ../../../v3d_main   
INCLUDEPATH +=  $$V3DMAINPATH/basic_c_fun
INCLUDEPATH +=  $$V3DMAINPATH/common_lib/include
INCLUDEPATH +=  $$V3DMAINPATH/v3d

HEADERS	+= eswc_converter_plugin.h
HEADERS	+= eswc_converter_func.h

SOURCES	=  eswc_converter_plugin.cpp
SOURCES	+= eswc_converter_func.cpp
SOURCES += eswc_core.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(eswc_converter)
DESTDIR = $$V3DMAINPATH/../bin/plugins//neuron_utilities/Enhanced_SWC_Format_Converter

