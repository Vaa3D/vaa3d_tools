
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off

mac{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    ITKLIBPATH = ../../../hackathon/zhi/snake_tracing/ITKlibs_MAC
}
else{
    ITKLIBPATH = ../../../hackathon/zhi/snake_tracing/ITKlibs_Linux
    SOURCES = ../../../hackathon/zhi/snake_tracing/ITK_include/itkLightProcessObject.cxx
}

VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/v3d $$VAA3DPATH/basic_c_fun $$VAA3DPATH/common_lib/include
INCLUDEPATH     += ../../../hackathon/zhi/snake_tracing/ITK_include


LIBS += -L$$ITKLIBPATH -litksys-4.5 -lITKCommon-4.5 -lITKStatistics-4.5 -lITKIOImageBase-4.5 -litkdouble-conversion-4.5
LIBS += -L$$ITKLIBPATH -lvnl_algo -lvnl -lv3p_netlib
LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat

HEADERS	+= Point.h
HEADERS += pixPoint.h
HEADERS += reconstruction_by_thinning_plugin_plugin.h
HEADERS += itkBinaryThinningImageFilter3D.h
SOURCES	+= Point.cxx
SOURCES += pixPoint.cxx
SOURCES += reconstruction_by_thinning_plugin_plugin.cpp
SOURCES += itkBinaryThinningImageFilter3D.txx
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

SOURCES += ../../../hackathon/zhi/snake_tracing/ITK_include/vcl_deprecated.cxx

TARGET	= $$qtLibraryTarget(SimpleAxisAnalyzer)
DESTDIR	= $$VAA3DPATH/../bin/plugins/bigneuronhackathon/SimpleAxisAnalyzer
