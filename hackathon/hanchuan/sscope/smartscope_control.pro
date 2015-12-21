# smartscope_control.pro
# a project that implements the V3D driver for SmartScope project
# by Hanchuan Peng
# 2011-May-13


V3DMAINPATH = ../../../../v3d_external

TEMPLATE      = lib
CONFIG       += qt plugin warn_off

INCLUDEPATH  += $$V3DMAINPATH/v3d_main/basic_c_fun 
INCLUDEPATH  += $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH  += DAQmx_ANSI_C_Dev/include
INCLUDEPATH  += FlyCapture2/include

LIBS         += -L$$V3DMAINPATH/v3d_main/common_lib/winlib -llibtiff
LIBS	     += -LDAQmx_ANSI_C_Dev/lib/msvc -lNIDAQmx
LIBS	     += -LFlyCapture2/lib -lFlyCapture2

HEADERS = smartscope_gui.h \
		smartscope_control.h \
		smartscope_lsm.h \
        configfileio_lsm.h \
		smartscope_dev_ni.h \
		smartscope_dev_pgr.h \
		smartscope_util.h \
		smartscope_ip.h \
		$$V3DMAINPATH/v3d_main/basic_c_fun/stackutil.h \
		$$V3DMAINPATH/v3d_main/basic_c_fun/mg_image_lib.h \
		$$V3DMAINPATH/v3d_main/basic_c_fun/mg_utilities.h \
		$$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.h
		
SOURCES = smartscope_gui.cpp \
		smartscope_control.cpp \
        smartscope_v3dplugin.cpp \
		smartscope_lsm.cpp \
        configfileio_lsm.cpp \
		smartscope_dev_ni.c \
		smartscope_util.cpp \
		smartscope_ip.cpp \
		$$V3DMAINPATH/v3d_main/basic_c_fun/stackutil.cpp \
		$$V3DMAINPATH/v3d_main/basic_c_fun/mg_image_lib.cpp \
		$$V3DMAINPATH/v3d_main/basic_c_fun/mg_utilities.cpp \
		$$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET  = $$qtLibraryTarget(smartscope_controller)
DESTDIR = $$V3DMAINPATH/bin/plugins/smartscope_controller

