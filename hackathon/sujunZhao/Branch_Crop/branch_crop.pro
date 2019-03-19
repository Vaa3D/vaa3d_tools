
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing
INCLUDEPATH	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_connector
HEADERS	+= branch_crop_plugin.h \
    branch_detect.h
#../../../../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h \
#../../../../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h\
#../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h

SOURCES	+= branch_crop_plugin.cpp \
    branch_detect.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp\
    ../../../../v3d_external/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/prune_short_branch.cpp/
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES +=../../../../vaa3d_tools/hackathon/PengXie/preprocess/sort_swc_redefined.cpp

TARGET	= $$qtLibraryTarget(branch_crop)
DESTDIR	= $$VAA3DPATH/bin/plugins/branch_crop/
