
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QT	+= widgets network
#CONFIG	+= x86_64
VAA3DRUNPATH = E:\Downloads\Vaa3D_v6.007_Windows_64bit\Vaa3D_v6.007_Windows_64bit
VAA3DPATH=D:\A_V3D\v3d_external_local
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= SuperPlugin_plugin.h \
    ../../v3d_external_local/v3d_main/basic_c_fun/basic_surf_objs.h \
    ../../vaa3d_tools/hackathon/zhi/APP2_large_scale/my_surf_objs.h \
    CallPlugin.h \
    DetachConnectedDomain.h \
    ExternalFunction.h \
    FileManage.h \
    GlobalConfig.h \
    ImageQuality.h \
    OutSource.h \
    PluginPipeline.h \
    SpLog.h \
    Task.h \
    TaskManage.h \
    WorkShop.h
SOURCES	+= SuperPlugin_plugin.cpp \
    ../../v3d_external_local/v3d_main/basic_c_fun/basic_surf_objs.cpp \
    ../../vaa3d_tools/hackathon/zhi/APP2_large_scale/my_surf_objs.cpp \
    CallPlugin.cpp \
    DetachConnectedDomain.cpp \
    ExternalFunction.cpp \
    FileManage.cpp \
    GlobalConfig.cpp \
    ImageQuality.cpp \
    OutSource.cpp \
    PluginPipeline.cpp \
    SpLog.cpp \
    Task.cpp \
    TaskManage.cpp \
    WorkShop.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(SuperPlugin)
DESTDIR	= $$VAA3DRUNPATH/plugins/SuperPlugin/
