
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH     += app2
INCLUDEPATH     += $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc

HEADERS	+= app2WithPreinfo_plugin.h \
    imgpreprocess.h \
    swc_convert.h \
    $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h \
    somefunction.h \
    app2.h
HEADERS += app2/fastmarching_tree.h
HEADERS += app2/hierarchy_prune.h
HEADERS += app2/fastmarching_dt.h

SOURCES	+= app2WithPreinfo_plugin.cpp \
    imgpreprocess.cpp \
    swc_convert.cpp \
    somefunction.cpp \
    app2.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += app2/my_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp

#meanshift
HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp

TARGET	= $$qtLibraryTarget(app2WithPreinfo)
DESTDIR	= $$VAA3DPATH/bin/plugins/app2WithPreinfo/
