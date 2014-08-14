
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include


HEADERS	= typeset_plugin.h
HEADERS	+= typeset_func.h
HEADERS += typeset.h
HEADERS += my_surf_objs.h
HEADERS += sort_func.h
HEADERS += openSWCDialog.h
HEADERS += sort_swc.h

HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/v3d_interface.h

SOURCES	= typeset_plugin.cpp
SOURCES	+= typeset_func.cpp
SOURCES += sort_func.cpp
SOURCES += openSWCDialog.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(typeset)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/typeset_childbranch
