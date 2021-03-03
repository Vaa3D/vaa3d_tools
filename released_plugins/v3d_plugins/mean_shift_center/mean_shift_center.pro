
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= \
    mean_shift_dialog.h \
    mean_shift_center_plugin.h \
    mean_shift_extr_template.h \
    ray_shoot_dialog.h \
    ../gsdt/main/heap.h \
    mean_shift_fun.h \
    gradient_transform_dialog.h \
    fastmarching_dt.h

SOURCES	+= \
    mean_shift_dialog.cpp \
    mean_shift_center_plugin.cpp \
    ray_shoot_dialog.cpp \
    mean_shift_fun.cpp \
    gradient_transform_dialog.cpp

SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(mean_shift_center_finder)
DESTDIR	= $$V3DMAINPATH/bin/plugins/image_analysis/mean_shift_center_finder/
