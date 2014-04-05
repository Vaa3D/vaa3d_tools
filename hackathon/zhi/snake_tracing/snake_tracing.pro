
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/v3d $$VAA3DPATH/basic_c_fun $$VAA3DPATH/common_lib/include
INCLUDEPATH     += /usr/local/lib/cmake/ITK-4.3
INCLUDEPATH     += /usr/local/share/vxl/cmake
INCLUDEPATH     += TracingCore
INCLUDEPATH     += /local2/neuron_tracing_comparision/farsight_build/ITK/Source/Modules/Filtering/Thresholding/include
INCLUDEPATH     += /local2/neuron_tracing_comparision/farsight-src/ftkImage
SRCLIBPATH = /local2/neuron_tracing_comparision/farsight_bin/libs
LIBS += -L$$SRCLIBPATH -lftkImage -lClusClus_lib -lftkGraphs -llibActiveValidation -lNuclear_Association -lsegment_soma -lTraceCoreLibrary
LIBS += -L$$SRCLIBPATH -lCytoplasmSegmentation -lftkGUI -llibagf -lNuclearSegmentation -lSPD -lYousef_Nucleus_Seg
LIBS += -L$$SRCLIBPATH -ldiffusion_map -lftkImage -llibSVM -lPixelLevelAnalysis -lsqlite3
LIBS += -L$$SRCLIBPATH -lftkCommon -lftkPreprocess -lMCLR -lProject_Processor -lTinyXML
LIBS += -L$$SRCLIBPATH -lftkFeatures -lftkSpectralUnmixing -lNESQLITE -lQHULL -lTrace




HEADERS	+= snake_tracing_plugin.h
SOURCES	+= snake_tracing_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

SOURCES += TracingCore/ImageOperation.cpp

TARGET	= $$qtLibraryTarget(snake_tracing)
DESTDIR	= ../../../../v3d_external/bin/plugins/snake_tracing/
