
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/vs2013project/vaa3d_tools
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= Subtree_Retrieval_plugin.h \
    split_subtree.h
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= Subtree_Retrieval_plugin.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp\
    split_subtree.cpp \
    subtree.cpp


TARGET	= $$qtLibraryTarget(Subtree_Retrieval)
DESTDIR	= $$VAA3DPATH/bin/plugins/Subtree_Retrieval/
