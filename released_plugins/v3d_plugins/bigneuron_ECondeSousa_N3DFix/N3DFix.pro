
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= N3DFix_plugin.h \
    Dend_Section.h \
    myHeader.h \
    openSWCDialog.h
SOURCES	+= N3DFix_plugin.cpp \
    remove_artifacts.cpp \
    main.cpp \
    Struct_from_swc.cpp \
    myFuncs.cpp \
    Load_and_Write_DATA.cpp \
    openSWCDialog.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(N3DFix)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_utilities/N3DFix/
