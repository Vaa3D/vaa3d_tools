
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include

HEADERS	+= open_fiji_plugin.h
SOURCES	+= open_fiji_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(open_fiji)
DESTDIR	= ../../../../v3d_external/bin/plugins/open_fiji

win32 {
        QMAKE_POST_LINK = copy Fiji.app ../../../../v3d_external/bin/
        QMAKE_POST_LINK = copy brl_FijiConvert.js ../../../../v3d_external/bin/

}
else {
        QMAKE_POST_LINK = cp -r Fiji.app ../../../../v3d_external/bin/ && cp brl_FijiConvert.js ../../../../v3d_external/bin/
        QMAKE_CLEAN += -r ../../../../v3d_external/bin/Fiji.app

}

