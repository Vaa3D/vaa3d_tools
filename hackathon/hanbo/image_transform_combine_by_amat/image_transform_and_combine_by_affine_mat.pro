
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /local3/hanbo/vaa3d/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= image_transform_and_combine_by_affine_mat_plugin.h
SOURCES	+= image_transform_and_combine_by_affine_mat_plugin.cpp \
    image_transform_and_combine_by_affine_mat_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(image_transform_and_combine_by_affine_mat)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_stitch/
