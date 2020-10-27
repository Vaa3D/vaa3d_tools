
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/penglab/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += ../../../../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2
INCLUDEPATH	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing
INCLUDEPATH	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_connector/

HEADERS	+= quality_control_plugin.h \
    feature_analysis.h\
../../../../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES	+= quality_control_plugin.cpp \
    feature_analysis.cpp \
    ../../../../v3d_external/released_plugins_more/v3d_plugins/neuron_connector/neuron_connector_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/prune_short_branch.cpp






TARGET	= $$qtLibraryTarget(quality_control)
DESTDIR	= $$VAA3DPATH/bin/plugins/quality_control/
