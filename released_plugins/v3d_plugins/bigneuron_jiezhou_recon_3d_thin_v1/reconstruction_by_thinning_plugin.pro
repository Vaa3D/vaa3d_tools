
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off

mac{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    ITKLIBPATH = ../bigneuron_zz_snake_tracing/ITKlibs_MAC
}
else{
    ITKLIBPATH = ../bigneuron_zz_snake_tracing/ITKlibs_Linux
    SOURCES = ../bigneuron_zz_snake_tracing/ITK_include/itkLightProcessObject.cxx
}

V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/v3d $$V3DMAINPATH/basic_c_fun $$V3DMAINPATH/common_lib/include
INCLUDEPATH     += ../bigneuron_zz_snake_tracing/ITK_include


LIBS += -L$$ITKLIBPATH -litksys-4.5 -lITKCommon-4.5 -lITKStatistics-4.5 -lITKIOImageBase-4.5 -litkdouble-conversion-4.5
LIBS += -L$$ITKLIBPATH -lvnl_algo -lvnl -lv3p_netlib
LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat

HEADERS	+= Point.h
HEADERS += pixPoint.h
HEADERS += reconstruction_by_thinning_plugin_plugin.h
HEADERS += itkBinaryThinningImageFilter3D.h
HEADERS += Edge.h
HEADERS += Vertex.h
HEADERS += Preprocessing.h
SOURCES	+= Point.cxx
SOURCES += pixPoint.cxx
SOURCES += Vertex.cxx
SOURCES += Edge.cxx
SOURCES += reconstruction_by_thinning_plugin_plugin.cpp
SOURCES += itkBinaryThinningImageFilter3D.txx
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

SOURCES += ../bigneuron_zz_snake_tracing/ITK_include/vcl_deprecated.cxx

TARGET	= $$qtLibraryTarget(SimpleAxisAnalyzer)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/neuron_tracing/SimpleAxisAnalyzer
