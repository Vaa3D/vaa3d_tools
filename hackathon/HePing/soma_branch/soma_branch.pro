
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/vs2013project/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= soma_branch_plugin.h \
    branch_count_soma.h
SOURCES	+= soma_branch_plugin.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp\
    branch_count_soma.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(soma_branch)
DESTDIR	= $$VAA3DPATH/bin/plugins/soma_branch/
