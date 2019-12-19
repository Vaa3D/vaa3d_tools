
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH  = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/
HEADERS	+= shift_mask_image_plugin.h \
    shift_mask2D.h \
    ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h
SOURCES	+= shift_mask_image_plugin.cpp \
    shift_mask2D.cpp
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_interface.h
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.h
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
#SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
TARGET	= $$qtLibraryTarget(shift_mask_image)
DESTDIR	= $$VAA3DPATH/bin/plugins/shift_mask_image/
