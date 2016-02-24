
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH  += $$VAA3DPATH/basic_c_fun
INCLUDEPATH  += $$VAA3DPATH/common_lib/include
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1
INCLUDEPATH     += $$VAA3DPATH/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/worm_straighten_c



HEADERS	+= neruoncrawler_app2_plugin.h
HEADERS	+= tracing_func.h

HEADERS	+= $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/readrawfile_func.h

HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app1.h

HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_tree.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/hierarchy_prune.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_dt.h

SOURCES	+= neruoncrawler_app2_plugin.cpp
SOURCES	+= tracing_func.cpp

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/stackutil.cpp

SOURCES	+= $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/readrawfile_func.cpp
SOURCES += $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/my_surf_objs.cpp

SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2_connector.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1_connector.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/v3dneuron_gd_tracing.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/gd.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/calculate_cover_scores.cpp

SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/swc_convert.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$$$VAA3DPATH/neuron_editing/v_neuronswc.cpp



TARGET	= $$qtLibraryTarget(neruoncrawler_app2)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_tracing/neruoncrawler_app2/
