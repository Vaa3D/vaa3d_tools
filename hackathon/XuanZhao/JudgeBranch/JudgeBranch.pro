
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include/
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2

HEADERS	+= JudgeBranch_plugin.h \
    randomforest.h \
    branchtree.h \
    judgebranchdialog.h \
    judgebranchfunction.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/swc_convert.h \
    $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h \
    ../../../released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h
SOURCES	+= JudgeBranch_plugin.cpp \
    randomforest.cpp \
    branchtree.cpp \
    judgebranchdialog.cpp \
    judgebranchfunction.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/swc_convert.cpp \
    $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(JudgeBranch)
DESTDIR	= $$VAA3DPATH/bin/plugins/JudgeBranch/
