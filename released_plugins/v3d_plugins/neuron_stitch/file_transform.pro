
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../..
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/neuron_editing

HEADERS	+= file_transform_plugin.h \
    neuron_stitch_func.h \
    $$VAA3DPATH/v3d_main/neuron_editing/neuron_xforms.h
SOURCES	+= file_transform_plugin.cpp \
    image_transform_and_combine_by_affine_mat_func.cpp \
    $$VAA3DPATH/v3d_main/neuron_editing/neuron_xforms.cpp \
    neuron_stitch_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(file_transform)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_stitch/3_file_transform/
