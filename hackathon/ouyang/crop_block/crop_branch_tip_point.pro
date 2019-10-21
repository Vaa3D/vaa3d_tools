
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH  = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
#INCLUDEPATH	+= $$VAA3DPATH/../vaa3d_tools/hackathon/ouyang/ML_get_samples/
INCLUDEPATH	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/
INCLUDEPATH	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_connector/
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include/
#INCLUDEPATH	+= $$VAA3DPATH/../vaa3d_tools/hackathon/yimin/old vr codes/Vaa3DVR(ver 0.1)/
unix {
    LIBS        += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
     }
HEADERS	+= crop_branch_tip_point_plugin.h \
    crop_block_func.h
SOURCES	+= crop_branch_tip_point_plugin.cpp \
    crop_block_func.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
#HEADERS	+= $$VAA3DPATH/../vaa3d_tools/hackathon/ouyang/ML_get_samples/tip_main.h
#SOURCES	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/prune_short_branch.cpp
#HEADERS	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/prune_short_branch.h
HEADERS	        += /home/braincenter5/vaa3d_tools/hackathon/yimin/old_vr_codes/Vaa3DVR(ver 0.1)/vrminimal/matrix.h

TARGET	= $$qtLibraryTarget(crop_branch_tip_point)
DESTDIR	= $$VAA3DPATH/bin/plugins/crop_branch_tip_point/

