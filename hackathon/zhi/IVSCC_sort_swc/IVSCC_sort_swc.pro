
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= IVSCC_sort_swc_plugin.h
HEADERS += openSWCDialog.h

SOURCES += openSWCDialog.cpp
SOURCES	+= IVSCC_sort_swc_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp

TARGET	= $$qtLibraryTarget(IVSCC_sort_swc)
DESTDIR	= $$VAA3DPATH/bin/plugins/IVSCC/IVSCC_sort_swc/
