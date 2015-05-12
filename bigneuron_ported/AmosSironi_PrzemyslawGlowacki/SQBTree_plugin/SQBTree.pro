
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

mac{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    ITKLIBPATH = ITKlibs_MAC
}
else{
    ITKLIBPATH = ITKlibs_Linux
    SOURCES = ITK_include/itkLightProcessObject.cxx
}

VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

#INCLUDEPATH	+= mex_related/
#INCLUDEPATH	+= /Applications/MATLAB_R2013a.app/extern/include
#INCLUDEPATH += $$_PRO_FILE_PWD_/sqb_0.1/include
#INCLUDEPATH += $$_PRO_FILE_PWD_/sqb_0.1/
INCLUDEPATH += liblbfgs-1.10/include
INCLUDEPATH	+= $$_PRO_FILE_PWD_/sqb_0.1/include
INCLUDEPATH += $$_PRO_FILE_PWD_/libconfig-1.4.9/lib
INCLUDEPATH += ITK_include
INCLUDEPATH += $$_PRO_FILE_PWD_/regression
INCLUDEPATH += $$_PRO_FILE_PWD_/sqb_0.1/src/MatrixSQB

LIBS += -L$$ITKLIBPATH -litksys-4.5 -lITKCommon-4.5 -lITKStatistics-4.5 -lITKIOImageBase-4.5 -litkdouble-conversion-4.5
LIBS += -L$$ITKLIBPATH -lvnl_algo -lvnl -lv3p_netlib

LIBS += -L"$$_PRO_FILE_PWD_/liblbfgs-1.10/lib"
LIBS += -llbfgs

LIBS += -L"$$_PRO_FILE_PWD_/libconfig-1.4.9/lib/.libs"
LIBS += -lconfig++

HEADERS	+= SQBTree_plugin.h
HEADERS += sqb_0.1/src/MatrixSQB/vaa3d_link.h
HEADERS += regression/regressor.h
HEADERS += regression/sep_conv.h
HEADERS += regression/regression_test2.h
HEADERS += sqb_0.1/src/MatrixSQB/sqb_trees.h

SOURCES += $$_PRO_FILE_PWD_/sqb_0.1/src/MatrixSQB/sqb_trees.cpp
SOURCES += $$_PRO_FILE_PWD_/regression/regression_test2.cpp

SOURCES	+= SQBTree_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= regression/regressor.cpp
SOURCES	+= regression/sep_conv.cpp
#SOURCES	+= regression/regression_test2.cpp
#SOURCES	+= sqb_0.1/src/MatrixSQB/sqb_trees.cpp

#QMAKE_CXXFLAGS += -DMEX
QMAKE_CXXFLAGS += -DLIBCONFIGXX_VER_REVISION
QMAKE_CXXFLAGS += -fPIC
#SOURCES += sqb_0.1/src/MatrixSQB/SQBTrees.cpp


#TARGET	= $$qtLibraryTarget(SQBTree)
TARGET	= $$qtLibraryTarget(SQBTree)
#TARGET	= sqb_tree
DESTDIR	= $$VAA3DPATH/bin/plugins/SQBTree/
