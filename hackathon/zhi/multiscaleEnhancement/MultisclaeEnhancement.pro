
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/jba/newmat11

LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat

INCLUDEPATH += main

HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h
HEADERS	+= MultisclaeEnhancement_plugin.h

SOURCES	+= MultisclaeEnhancement_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(MultisclaeEnhancement)
DESTDIR	= ../../../bin/plugins/MultisclaeEnhancement
