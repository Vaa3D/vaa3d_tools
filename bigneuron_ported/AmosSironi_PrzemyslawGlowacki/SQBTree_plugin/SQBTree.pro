
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

#INCLUDEPATH	+= mex_related/
#INCLUDEPATH	+= /Applications/MATLAB_R2013a.app/extern/include
INCLUDEPATH += $$_PRO_FILE_PWD_/sqb_0.1/include
INCLUDEPATH += liblbfgs-1.10/include
#INCLUDEPATH	+= ../sqb_0.1/include/SQB/Core
INCLUDEPATH += $$_PRO_FILE_PWD_/libconfig-1.4.9/lib

LIBS += -L"$$_PRO_FILE_PWD_/liblbfgs-1.10/lib"
LIBS += -llbfgs

LIBS += -L"$$_PRO_FILE_PWD_/libconfig-1.4.9/lib"
LIBS += -lconfig++

HEADERS	+= SQBTree_plugin.h
HEADERS += sqb_0.1/src/MatrixSQB/vaa3d_link.h


SOURCES	+= SQBTree_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

#QMAKE_CXXFLAGS += -DMEX
QMAKE_CXXFLAGS += -DLIBCONFIGXX_VER_REVISION
#SOURCES += sqb_0.1/src/MatrixSQB/SQBTrees.cpp
SOURCES += sqb_0.1/src/MatrixSQB/sqb_trees.cpp


#TARGET	= $$qtLibraryTarget(SQBTree)
TARGET	= sqb_tree
DESTDIR	= $$VAA3DPATH/bin/plugins/SQBTree/
