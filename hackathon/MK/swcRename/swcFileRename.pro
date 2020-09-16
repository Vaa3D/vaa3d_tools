
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/Vaa3D_2013_Qt486/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/v3d_main/v3d
INCLUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH += $$(BOOST_PATH)

HEADERS	+= swcFileRename_plugin.h
HEADERS += swcRenameDlg.h

SOURCES	+= swcFileRename_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += swcRenameDlg.cpp

FORMS += renameSWC.ui

TARGET	= $$qtLibraryTarget(swcFileRename)
DESTDIR	= $$VAA3DPATH/bin/plugins/swcFileRename/
