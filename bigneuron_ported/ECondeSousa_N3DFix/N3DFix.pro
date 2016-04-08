
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = C:/Vaa3d/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

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
    ../../../../../Vaa3d/v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(N3DFix)
DESTDIR	= C:/Vaa3d/build_cmake/v3d/Windows_MSVC_64/plugins/N3DFix/
