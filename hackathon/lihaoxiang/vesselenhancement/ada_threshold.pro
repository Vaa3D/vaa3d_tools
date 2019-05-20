
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += main 

macx{
    LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}

win32 {
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib64 -llibtiff
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib64 -llibnewmat
}

HEADERS += ada_threshold.h
SOURCES  = ada_threshold.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(ada_threshold)
DESTDIR       = $$V3DMAINPATH/../bin/plugins/image_thresholding/Simple_Adaptive_Thresholding_test
