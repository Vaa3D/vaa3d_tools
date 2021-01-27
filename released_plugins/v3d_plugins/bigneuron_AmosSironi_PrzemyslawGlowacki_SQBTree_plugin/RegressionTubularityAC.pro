
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

mac{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    ITKLIBPATH = ./libs/ITKlibs_MAC
}
else{
    ITKLIBPATH = ./libs/ITKlibs_Linux
    SOURCES = ./libs/ITK_include/itkLightProcessObject.cxx
}

V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d


#INCLUDEPATH += ./libs/liblbfgs-1.10/include
INCLUDEPATH += ./libs/liblbfgs-1.10_include
INCLUDEPATH += ./libs/sqb_0.1/include
#INCLUDEPATH += ./libs/libconfig-1.4.9/lib
INCLUDEPATH += ./libs/libconfig-1.4.9_include
INCLUDEPATH += ./libs/ITK_include
INCLUDEPATH += ./libs/regression
INCLUDEPATH += ./libs/sqb_0.1/src/MatrixSQB
INCLUDEPATH += ./libs/boost_1_58_0
INCLUDEPATH     += $$V3DMAINPATH/v3d_main/common_lib/include


LIBS += -L$$ITKLIBPATH -litksys-4.5 -lITKCommon-4.5 -lITKStatistics-4.5 -lITKIOImageBase-4.5 -litkdouble-conversion-4.5
LIBS += -L$$ITKLIBPATH -lvnl_algo -lvnl -lv3p_netlib

LIBS += -L$$V3DMAINPATH/v3d_main/common_lib/lib -lv3dtiff



#LIBS += -L"./libs/liblbfgs-1.10/lib/.libs"
LIBS += -L"./libs/liblbfgs-1.10_Linux/lib"
LIBS += -llbfgs

#LIBS += -L"./libs/libconfig-1.4.9/lib/"
#LIBS += -L"./libs/libconfig-1.4.9_old/lib/.libs"
LIBS += -L"./libs/libconfig-1.4.9_Linux"
LIBS += -lconfig++


HEADERS += ./libs/sqb_0.1/src/MatrixSQB/vaa3d_link.h
HEADERS += ./libs/regression/sep_conv.h
HEADERS += ./libs/regression/util.h
HEADERS += ./libs/regression/sampling.h
HEADERS += ./libs/regression/regressor.h

HEADERS	+= RegressionTubularityAC_plugin.h


SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= ./libs/regression/sep_conv.cpp
SOURCES	+= ./libs/regression/sampling.cpp
SOURCES	+= ./libs/regression/regressor.cpp
SOURCES	+= RegressionTubularityAC_plugin.cpp

QMAKE_CXXFLAGS += -DLIBCONFIGXX_VER_REVISION
QMAKE_CXXFLAGS += -fPIC
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
#QMAKE_CXXFLAGS += -std=c++11


TARGET	= $$qtLibraryTarget(RegressionTubularityAC)
DESTDIR	= $$V3DMAINPATH/bin/plugins/image_segmentation/RegressionTubularityAC/
