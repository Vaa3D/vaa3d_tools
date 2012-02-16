TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
CONFIG  -= bundle_off
V3DMAINPATH  =  ../../../v3d_external/v3d_main   
INCLUDEPATH +=  $$V3DMAINPATH/basic_c_fun

HEADERS	+= eswc_converter_plugin.h
HEADERS	+= eswc_converter_func.h

SOURCES	=  eswc_converter_plugin.cpp
SOURCES	+= eswc_converter_func.cpp
SOURCES += eswc_core.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(eswc_converter)
