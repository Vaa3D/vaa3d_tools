#an test plugin project file
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off

#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main

#include necessary paths
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include


#LIBS += -L. -lv3dtiff -L$$VAA3DPATH/commom_lib/lib
#LIBS         += -lm -L$$VAA3DPATH/common_lib/lib -lv3dtiff
#LIBS 	     += -L$$VAA3DPATH/common_lib/lib -lv3dtiff


#include the headers used in the project
HEADERS	+= test_plugin.h
HEADERS += test_func.h

#include the source files used in the project
SOURCES	+= test_plugin.cpp \
    test_func.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/stackutil.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_memory.cpp

#specify target name and directory
TARGET	= $$qtLibraryTarget(test)
DESTDIR	= $$VAA3DPATH/bin/plugins/test/
