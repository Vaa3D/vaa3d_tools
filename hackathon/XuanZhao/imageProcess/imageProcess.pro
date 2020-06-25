
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= imageProcess_plugin.h \
    imageprocess.h

HEADERS	+= $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h

SOURCES	+= imageProcess_plugin.cpp \
    imageprocess.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.cpp

TARGET	= $$qtLibraryTarget(imageProcess)
DESTDIR	= $$VAA3DPATH/bin/plugins/imageProcess/
