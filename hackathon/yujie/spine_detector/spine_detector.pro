
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH += ../../../released_plugins/v3d_plugins/neurontracing_vn2
INCLUDEPATH += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2


HEADERS	+= spine_detector_plugin.h \
    spine_detector_dialog.h \
    detect_fun.h \
    spine_extr_template.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/heap.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_dt.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_tree.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.h \
    #../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h \
    app2.h \
    spine_fun.h

SOURCES	+= spine_detector_plugin.cpp \
    spine_detector_dialog.cpp \
    detect_fun.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.cpp \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp \
    #../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp \
    app2.cpp \
    spine_fun.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp \
           $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp \
           $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(spine_detector)
DESTDIR	= $$VAA3DPATH/bin/plugins/spine_detector/
