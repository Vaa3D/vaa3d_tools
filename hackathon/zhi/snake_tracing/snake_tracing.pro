
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off

VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/v3d $$VAA3DPATH/basic_c_fun $$VAA3DPATH/common_lib/include
INCLUDEPATH     += ITK

ITKLIBPATH = libs

LIBS += -L$$ITKLIBPATH -litksys-4.3 -lITKCommon-4.3 -lITKStatistics-4.3 -lITKIOImageBase-4.3 -lITKVNLInstantiation-4.3 -lc++ #-lvnl_algo -lvnl

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
           ITK/itkRegion.cxx\
           ITK/itkLightObject.cxx\
           ITK/itkDataObject.cxx\
           ITK/itkExceptionObject.cxx\
           ITK/itkMetaDataDictionary.cxx\
           ITK/itkObject.cxx\
           ITK/itkProcessObject.cxx\
           ITK/itkRealTimeStamp.cxx\
           ITK/itkIndent.cxx\
           ITK/itkImageIORegion.cxx\
           ITK/vcl_deprecated.cxx\
           ITK/itkEventObject.cxx


TARGET	= $$qtLibraryTarget(snake_tracing)
DESTDIR	= $$VAA3DPATH/../bin/plugins/snake_tracing/
