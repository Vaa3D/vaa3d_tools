
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off

mac{
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
}

VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/v3d $$VAA3DPATH/basic_c_fun $$VAA3DPATH/common_lib/include
INCLUDEPATH     += ITK4.5
INCLUDEPATH     +=  TracingCore/Filters/

ITKLIBPATH = ITKlibs
LIBS += -L$$ITKLIBPATH -litksys-4.5 -lITKCommon-4.5 -lITKStatistics-4.5 -lITKIOImageBase-4.5 -litkdouble-conversion-4.5
LIBS += -L$$ITKLIBPATH -lvnl_algo -lvnl -lv3p_netlib
LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat

HEADERS	+= snake_tracing_plugin.h
SOURCES	+= snake_tracing_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

SOURCES += TracingCore/ImageOperation.cpp\
           TracingCore/PointOperation.cpp\
           TracingCore/SnakeOperation.cpp\
           OpenSnakeTracer.cpp\
           ITK4.5/vcl_deprecated.cxx

TARGET	= $$qtLibraryTarget(snake_tracing)
DESTDIR	= $$VAA3DPATH/../bin/plugins/snake_tracing/
