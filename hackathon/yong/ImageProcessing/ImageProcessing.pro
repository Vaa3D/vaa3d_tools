
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= ImageProcessing_plugin.h \
    threshold_select.h \
    GMM_Algorithm.h \
    binary_gsdt.h \
    FM/fastmarching_dt.h \
    FM/heap.h \
    binary_connected.h \
    get_sub_block.h \
    branch_angle.h \
    expand_swc.h \
    ImageGrayValue.h \
    ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h \
    image_label.h \
    v3drawTOtif.h
SOURCES	+= ImageProcessing_plugin.cpp \
    threshold_select.cpp \
    GMM_Algorithm.cpp \
    binary_gsdt.cpp \
    binary_connected.cpp \
    get_sub_block.cpp \
    branch_angle.cpp \
    expand_swc.cpp \
    ImageGrayValue.cpp \
    ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp \
    image_label.cpp \
    v3drawTOtif.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(ImageProcessing)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/ImageProcessing/


INCLUDEPATH += /usr/local/opencv3.1.0//include \
                /usr/local/opencv3.1.0/include/opencv \
                /usr/local/opencv3.1.0/include/opencv2

LIBS += /usr/local/opencv3.1.0/lib/libopencv_highgui.so \
        /usr/local/opencv3.1.0/lib/libopencv_core.so    \
        /usr/local/opencv3.1.0/lib/libopencv_imgproc.so
