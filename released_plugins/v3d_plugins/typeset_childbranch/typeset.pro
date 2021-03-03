
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d


HEADERS	= typeset_plugin.h
HEADERS	+= typeset_func.h
HEADERS += typeset.h
HEADERS += my_surf_objs.h
HEADERS += sort_func.h
HEADERS += openSWCDialog.h
HEADERS += sort_swc.h

HEADERS += $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_interface.h

SOURCES	= typeset_plugin.cpp
SOURCES	+= typeset_func.cpp
SOURCES += sort_func.cpp
SOURCES += openSWCDialog.cpp

SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(typeset)
#DESTDIR = C:\Users\rummig\Desktop\Vaa3d_allfiles\vaa3d_win7_32bit_v2.707\plugins\typeset_childbranch #to test
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_utilities/typeset_childbranch
