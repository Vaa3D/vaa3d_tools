
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QT += core gui widgets opengl openglwidgets
#CONFIG	+= x86_64
VAA3DPATH = D:\A_V3D\v3d_external_local
RUNV3DPATH = C:\Users\admin\Downloads\Vaa3D_v6.007_Windows_64bit\Vaa3D_v6.007_Windows_64bit
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/v3d
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/plugin_loader
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/3drenderer
INCLUDEPATH     += $$V3DMAINPATH

HEADERS	+= SuperPlugin_plugin.h \
    Computation.h \
    Postprocess.h \
    Preprocess.h \
    command.h \
    data_io.h \
    superplugin_ui.h
SOURCES	+= SuperPlugin_plugin.cpp \
    Computation.cpp \
    Postprocess.cpp \
    Preprocess.cpp \
    command.cpp \
    data_io.cpp \
    superplugin_ui.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(SuperPlugin)
DESTDIR	= $$RUNV3DPATH/plugins/SuperPlugin/
