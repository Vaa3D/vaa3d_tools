
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off

mac{
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
    ITKLIBPATHONE = ../../../hackathon/zhi/snake_tracing/ITKlibs_MAC
    ITKLIBPATHTWO = ./lib_ITK4.3
}
else{
    ITKLIBPATH = ../../../hackathon/zhi/snake_tracing/ITKlibs_Linux
    SOURCES = ../../../hackathon/zhi/snake_tracing/ITK_include/itkLightProcessObject.cxx
}

VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/v3d $$VAA3DPATH/basic_c_fun $$VAA3DPATH/common_lib/include
INCLUDEPATH     += ../../../hackathon/zhi/snake_tracing/ITK_include


LIBS += -L$$ITKLIBPATHONE -litksys-4.5 -lITKCommon-4.5 -lITKStatistics-4.5 -lITKIOImageBase-4.5 -litkdouble-conversion-4.5
LIBS += -L$$ITKLIBPATHONE -lvnl_algo -lvnl -lv3p_netlib

#LIBS += -L$$ITKLIBPATH -litksys-4.3 -lITKCommon-4.3 -lITKStatistics-4.3 -lITKIOImageBase-4.3 #-litkdouble-conversion-4.3
#LIBS += -L$$ITKLIBPATH -lvnl -lv3p_netlib
LIBS += -L$$ITKLIBPATHTWO  -lITKIOJPEG-4.3 -litkjpeg-4.3 -litkopenjpeg-4.3 -litkgdcmjpeg12-4.3 -litkgdcmjpeg16-4.3 -litkgdcmjpeg8-4.3

LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat

HEADERS	+= Point.h
HEADERS += pixPoint.h
HEADERS += reconstruction_by_thinning_plugin_plugin.h
HEADERS += itkBinaryThinningImageFilter3D.h
HEADERS += Edge.h
HEADERS += Vertex.h
HEADERS += Preprocessing.h
#HEADERS += Reconstruction3D.cxx

SOURCES	+= Point.cxx
SOURCES += pixPoint.cxx
SOURCES += Vertex.cxx
SOURCES += Edge.cxx
SOURCES += reconstruction_by_thinning_plugin_plugin.cpp
SOURCES += itkBinaryThinningImageFilter3D.txx
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

SOURCES += ../../../hackathon/zhi/snake_tracing/ITK_include/vcl_deprecated.cxx

TARGET	= $$qtLibraryTarget(SimpleAxisAnalyzer)
DESTDIR	= $$VAA3DPATH/../bin/plugins/bigneuronhackathon/SimpleAxisAnalyzer
