
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include/
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2
INCLUDEPATH     += $$VAA3DPATH/v3d_main/v3d
INCLUDEPATH     += $$VAA3DPATH/v3d_main/jba/newmat11
INCLUDEPATH     += $$VAA3DPATH/../vaa3d_tools/hackathon/XuanZhao/JudgeBranch/vigra

unix {
LIBS += -L$$VAA3DPATH/v3d_main/jba/c++
LIBS += -lv3dnewmat
}
win32 {
LIBS += -L$$VAA3DPATH/v3d_main/common_lib/winlib64
LIBS += -llibnewmat
}

HEADERS	+= JudgeBranch_plugin.h \
    randomforest.h \
    branchtree.h \
    judgebranchdialog.h \
    judgebranchfunction.h \
#    ../../../released_plugins/v3d_plugins/neurontracing_vn2/swc_convert.h \
    $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h \
#    ../../../released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h \
    $$VAA3DPATH/v3d_main/v3d/compute_win_pca.h \
    $$VAA3DPATH/v3d_main/basic_c_fun/basic_memory.h \
    filter.h \
    pixelclassification.h \
    swc_convert.h \
    swc2mask.h



SOURCES	+= JudgeBranch_plugin.cpp \
    randomforest.cpp \
    branchtree.cpp \
    judgebranchdialog.cpp \
    judgebranchfunction.cpp \
    $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp \
    $$VAA3DPATH/v3d_main/basic_c_fun/basic_memory.cpp \
    filter.cpp \
    pixelclassification.cpp \
    swc_convert.cpp \
    swc2mask.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(JudgeBranch)
DESTDIR	= $$VAA3DPATH/bin/plugins/JudgeBranch/
