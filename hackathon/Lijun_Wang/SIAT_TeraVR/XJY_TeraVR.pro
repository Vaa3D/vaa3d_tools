include(../terafly/terafly.pro)


CONFIG	+= qt plugin warn_off
#CONFIG += console
TEMPLATE = lib
DEFINES += __NAWEBSERVICE__
DEFINES += _ALLOW_WORKMODE_MENU_
DEFINES += __ALLOW_VR_FUNCS__
#CONFIG +=  warn_off thread


#CONFIG	+= x86_64
VAA3DPATH = C:\Users\82700\v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include/hdf5
INCLUDEPATH += $$VAA3DPATH/v3d_main/v3d
INCLUDEPATH += $$VAA3DPATH/v3d_main/3drenderer
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun/customary_structs
INCLUDEPATH += $$VAA3DPATH/v3d_main/vrrenderer

LIBS += -LC:/Users/82700/v3d_external/v3d_main/common_lib/mingw64  -lhdf5 -lszip -lzlib  -lteem  -lz -lv3dtiff -lv3dnewmat -lOpengl32  -lglu32 -lwsock32
QT += core gui widgets opengl openglwidgets network xml svg
HEADERS	+= XJY_TeraVR_plugin.h
SOURCES	+= XJY_TeraVR_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(SIAT_TeraVR)
DESTDIR	= $$VAA3DPATH/bin/plugins/SIAT_TeraVR/
