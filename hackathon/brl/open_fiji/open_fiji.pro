
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
DESTDIR	= $$V3DMAINPATH/../../v3d_external/bin/plugins/open_fiji

win32 {
        QMAKE_POST_LINK = copy Fiji.app ../../../../v3d_external/bin/
        QMAKE_POST_LINK = copy brl_FijiConvert.js ../../../../v3d_external/bin/

}
else {    #  copy the Fiji app into the bin directory and then delete the .svn stuff
        QMAKE_POST_LINK = cp -r Fiji.app $$V3DMAINPATH/../../v3d_external/bin/ &&  cp brl_FijiConvert.js $$V3DMAINPATH/../../v3d_external/bin/ && find $$V3DMAINPATH/../../v3d_external/bin/ -name "*.svn" | xargs rm -r

        QMAKE_CLEAN += -r $$V3DMAINPATH/../../v3d_external/bin/Fiji.app
}

