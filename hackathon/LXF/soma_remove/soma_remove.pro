
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= soma_remove_plugin.h \
    soma_remove_main.h \
    my_surf_objs.h \
    sort_swc.h \
    data_training.h \
    matrix.h \
    QR.h \
    SVD.h \
    general.h \
    fun.h \
    svds.h \
    openfileDialog.h
HEADERS += ../../heyishan/blastneuron_bjut/overlap/overlap_gold.h
HEADERS	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES	+= soma_remove_plugin.cpp \
    soma_remove_main.cpp \
    my_surf_objs.cpp \
    data_training.cpp \
    QR.cpp \
    matrix.cpp \
    general.cpp \
    svds.cpp \
    openfileDialog.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += ../../heyishan/blastneuron_bjut/overlap/overlap_gold.cpp

TARGET	= $$qtLibraryTarget(soma_remove)
DESTDIR	= $$VAA3DPATH/bin/plugins/soma_remove/
