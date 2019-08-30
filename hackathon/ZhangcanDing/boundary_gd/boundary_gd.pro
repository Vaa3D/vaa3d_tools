
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/penglab/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/mean_shift_center
INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/neurontracing_vn2/app2

HEADERS	+= boundary_gd_plugin.h \
    boundary_gd_func.h
SOURCES	+= boundary_gd_plugin.cpp \
    boundary_gd_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(boundary_gd)
DESTDIR	= $$VAA3DPATH/bin/plugins/bigneuronhackathon/boundary_gd/
