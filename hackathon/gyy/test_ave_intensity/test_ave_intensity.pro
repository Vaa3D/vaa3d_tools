
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/balala/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= test_ave_intensity_plugin.h \
    ../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h

SOURCES	+= test_ave_intensity_plugin.cpp \
    ../../../released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(test_ave_intensity)
DESTDIR	= $$VAA3DPATH/bin/plugins/test_ave_intensity/
