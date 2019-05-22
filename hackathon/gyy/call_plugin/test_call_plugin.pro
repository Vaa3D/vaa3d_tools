
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/balala/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include


HEADERS	+= test_call_plugin_plugin.h

HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS +=  ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_dt.h
HEADERS +=  ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_tree.h
HEADERS +=  ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/hierarchy_prune.h
SOURCES	+= test_call_plugin_plugin.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2_connector.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.cpp
TARGET	= $$qtLibraryTarget(test_call_plugin)
DESTDIR	= $$VAA3DPATH/bin/plugins/test_call_plugin/

FORMS +=
