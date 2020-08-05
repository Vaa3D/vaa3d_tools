
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2
INCLUDEPATH     += $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc

HEADERS	+= app2WithPreinfo_plugin.h \
    app2.h \
    imgpreprocess.h \
    swc_convert.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_dt.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_tree.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/hierarchy_prune.h \
    $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h \
    somefunction.h
SOURCES	+= app2WithPreinfo_plugin.cpp \
    app2.cpp \
    imgpreprocess.cpp \
    swc_convert.cpp \
    somefunction.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp

TARGET	= $$qtLibraryTarget(app2WithPreinfo)
DESTDIR	= $$VAA3DPATH/bin/plugins/app2WithPreinfo/
