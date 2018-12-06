
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/braincenter10/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= ML_get_sample_plugin.h \
    get_tip_block.h
SOURCES	+= ML_get_sample_plugin.cpp \
    get_tip_block.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(ML_get_sample)
DESTDIR	= $$VAA3DPATH/bin/plugins/ML_get_sample/
