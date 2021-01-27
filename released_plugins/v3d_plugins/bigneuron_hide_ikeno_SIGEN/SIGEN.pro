V3DMAINPATH = ../../../../v3d_external

TEMPLATE = lib
CONFIG += qt plugin release

TARGET = $$qtLibraryTarget(SIGEN)

# http://stackoverflow.com/questions/2580934
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_tracing/SIGEN/

unix{
    QMAKE_CXXFLAGS += -Wall -Wextra -Wshadow -Wno-c++11-extensions
}

INCLUDEPATH += $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/v3d_main/v3d
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

HEADERS += SIGEN_plugin.h
SOURCES += SIGEN_plugin.cpp

INCLUDEPATH += src
INCLUDEPATH += third_party
SOURCES += src/sigen/interface.cpp
SOURCES += src/sigen/builder/builder.cpp
SOURCES += src/sigen/common/disjoint_set.cpp
SOURCES += src/sigen/common/neuron.cpp
SOURCES += src/sigen/extractor/extractor.cpp
SOURCES += src/sigen/toolbox/toolbox.cpp
SOURCES += third_party/kdtree/kdtree.c
