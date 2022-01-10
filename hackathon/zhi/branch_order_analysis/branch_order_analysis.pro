
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= branch_order_analysis_plugin.h
SOURCES	+= branch_order_analysis_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/pruning_swc/my_surf_objs.cpp


TARGET	= $$qtLibraryTarget(branch_order_analysis)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/branch_order_analysis/
