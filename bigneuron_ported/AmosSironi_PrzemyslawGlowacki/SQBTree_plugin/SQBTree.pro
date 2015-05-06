
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /Users/pengh/work/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

#INCLUDEPATH	+= ../sqb_0.1/include/SQB/Core


HEADERS	+= SQBTree_plugin.h

HEADERS += ../sqb_0.1/src/MatrixSQB/vaa3d_link.h


SOURCES	+= SQBTree_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp


SOURCES += ../sqb_0.1/src/MatrixSQB/SQBTrees.cpp


TARGET	= $$qtLibraryTarget(SQBTree)
DESTDIR	= $$VAA3DPATH/bin/plugins/SQBTree/
