
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

mac{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    ITKLIBPATH = ../libs/ITKlibs_MAC
}
else{
    ITKLIBPATH = ../libs/ITKlibs_Linux
    SOURCES = ../libs/ITK_include/itkLightProcessObject.cxx
}

VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

#INCLUDEPATH	+= mex_related/
#INCLUDEPATH	+= /Applications/MATLAB_R2013a.app/extern/include
#INCLUDEPATH += $$_PRO_FILE_PWD_/sqb_0.1/include
#INCLUDEPATH += $$_PRO_FILE_PWD_/sqb_0.1/
INCLUDEPATH += ../libs/liblbfgs-1.10/include
INCLUDEPATH += ../libs/sqb_0.1/include
INCLUDEPATH += ../libs/libconfig-1.4.9/lib
INCLUDEPATH += ../libs/ITK_include
INCLUDEPATH += ../libs/regression
INCLUDEPATH += ../libs/sqb_0.1/src/MatrixSQB
INCLUDEPATH += ../libs/boost_1_58_0 # download and unzip it (not in svn because it is big)

LIBS += -L$$ITKLIBPATH -litksys-4.5 -lITKCommon-4.5 -lITKStatistics-4.5 -lITKIOImageBase-4.5 -litkdouble-conversion-4.5
LIBS += -L$$ITKLIBPATH -lvnl_algo -lvnl -lv3p_netlib

LIBS += -L"../libs/liblbfgs-1.10/lib"
LIBS += -llbfgs

LIBS += -L"../libs/libconfig-1.4.9/lib/.libs"
LIBS += -lconfig++

HEADERS += ../libs/sqb_0.1/src/MatrixSQB/vaa3d_link.h
#HEADERS += sqb_0.1/include/SQB/Core/Booster.h
#HEADERS += ../libs/sqb_0.1/src/MatrixSQB/sqb_trees.h
HEADERS += ../libs/regression/sep_conv.h
HEADERS += ../libs/regression/util.h
HEADERS += ../libs/regression/sampling.h
HEADERS += ../libs/regression/regressor.h

HEADERS	+= RegressionTubularityAC_plugin.h



#SOURCES += $$_PRO_FILE_PWD_/regression/regression_test2.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
#SOURCES	+= regression/regressor.cpp
SOURCES	+= ../libs/regression/sep_conv.cpp
SOURCES	+= ../libs/regression/sampling.cpp
SOURCES	+= ../libs/regression/regressor.cpp
#SOURCES	+= regression/regression_test2.cpp
#SOURCES	+= sqb_0.1/src/MatrixSQB/sqb_trees.cpp
SOURCES	+= RegressionTubularityAC_plugin.cpp

#QMAKE_CXXFLAGS += -DMEX
QMAKE_CXXFLAGS += -DLIBCONFIGXX_VER_REVISION
QMAKE_CXXFLAGS += -fPIC
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
#QMAKE_CXXFLAGS +=-lboost_system
#QMAKE_CXXFLAGS += -std=c++11
#SOURCES += sqb_0.1/src/MatrixSQB/SQBTrees.cpp


#TARGET	= $$qtLibraryTarget(SQBTree)
TARGET	= $$qtLibraryTarget(RegressionTubularityAC)
#TARGET	= sqb_tree
DESTDIR	= $$VAA3DPATH/bin/plugins/RegressionTubularityAC/
