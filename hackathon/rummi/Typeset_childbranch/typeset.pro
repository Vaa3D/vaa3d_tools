
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = C:\Users\rummig\Desktop\Vaa3d_allfiles\Vaa3d_Downloads\v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= typeset_plugin.h \
    typeset_func.h \
    typeset.h

SOURCES	+= typeset_plugin.cpp \
    typeset_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(typeset)
DESTDIR	= C:\Users\rummig\Desktop\Vaa3d_allfiles\vaa3d_win7_32bit_v2.707\plugins\typeset_childbranch
