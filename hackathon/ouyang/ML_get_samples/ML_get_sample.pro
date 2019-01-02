
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= ML_get_sample_plugin.h \
    get_tip_block.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h \
    APP1_pruning.h
SOURCES	+= ML_get_sample_plugin.cpp \
    get_tip_block.cpp \
    APP1_pruning.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

HEADERS	+= $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h
SOURCES	+= $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.cpp

TARGET	= $$qtLibraryTarget(ML_get_sample)
DESTDIR	= $$VAA3DPATH/bin/plugins/ML_get_sample/
