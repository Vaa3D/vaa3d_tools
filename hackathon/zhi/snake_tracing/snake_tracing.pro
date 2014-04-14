
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64


VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/v3d $$VAA3DPATH/basic_c_fun $$VAA3DPATH/common_lib/include
INCLUDEPATH     += ITK_4.3

ITKLIBPATH = libs2
LIBS += -L$$ITKLIBPATH -litksys-4.6 -lITKCommon-4.6 -lITKStatistics-4.6 -lITKIOImageBase-4.6
LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat

HEADERS	+= snake_tracing_plugin.h
SOURCES	+= snake_tracing_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

SOURCES += TracingCore/ImageOperation.cpp\
           TracingCore/PointOperation.cpp\
           TracingCore/SnakeOperation.cpp\
           OpenSnakeTracer.cpp\
           ITK_4.3/itkLightProcessObject.cxx

TARGET	= $$qtLibraryTarget(snake_tracing)
DESTDIR	= ../../../../v3d_external/bin/plugins/snake_tracing/
