
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64


GSLLIBPATH = gsl_lib_linux


V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += gsl_include

INCLUDEPATH += main 

LIBS += $$GSLLIBPATH/libgsl.a

HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h
HEADERS += SteerableFilter3Dplugin.h
HEADERS += steerableDetector.h
HEADERS += convolver.h
SOURCES  = SteerableFilter3Dplugin.cpp

SOURCES += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += steerableDetector.cpp
SOURCES += convolver.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp

TARGET        = $$qtLibraryTarget(SteerableFilter3D)
DESTDIR       = $$V3DMAINPATH/../bin/plugins/image_filters/SteerableFilter3D

