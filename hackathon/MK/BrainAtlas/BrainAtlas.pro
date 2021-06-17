
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/AllenVaa3D_2013_Qt486/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/v3d_main/v3d
INCLUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH += D:/boostLibs/boost_1_72_0
INCLUDEPATH += ../v3d_imgManagerMK

HEADERS	+= BrainAtlas_plugin.h
HEADERS += BrainAtlasControlPanel.h

SOURCES	+= BrainAtlas_plugin.cpp
SOURCES += BrainAtlasControlPanel.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

FORMS += brainRegionListTest.ui

TARGET	= $$qtLibraryTarget(BrainAtlas)
DESTDIR	= $$VAA3DPATH/bin/plugins/BrainAtlas/
