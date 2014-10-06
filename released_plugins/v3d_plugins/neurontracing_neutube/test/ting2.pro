
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QT += opengl xml
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_main
SRCNEUTUBEPATH = ../src_neutube

INCLUDEPATH += $$VAA3DPATH/v3d $$VAA3DPATH/basic_c_fun $$VAA3DPATH/common_lib/include   $$SRCNEUTUBEPATH/neurolabi/c \
    $$SRCNEUTUBEPATH/neurolabi/c/include $$SRCNEUTUBEPATH/neurolabi/gui \
    $$SRCNEUTUBEPATH/neurolabi/lib/genelib/src $$SRCNEUTUBEPATH/neurolabi/build \
    $$SRCNEUTUBEPATH/neurolabi/gui/ext $$SRCNEUTUBEPATH/neurolabi/gui/ext/QsLog

INCLUDEPATH += $$SRCNEUTUBEPATH/neurolabi/lib/libpng-1.6.7 $$SRCNEUTUBEPATH/neurolabi/lib/jansson/include


LIBS += -L$${PWD}/$$SRCNEUTUBEPATH/neurolabi/build -lneutube -L$${PWD}/$$SRCNEUTUBEPATH/neurolabi/c/lib -lneurolabi \
    -lxml2 -ljansson -lGLEW -framework AGL -framework OpenGL -lfftw3 -lfftw3f -ldl \
    -framework AppKit -framework IOKit     -framework OpenGL \
    -framework ApplicationServices -ldl -framework AppKit -framework IOKit \
    -framework OpenGL -framework ApplicationServices -framework CoreFoundation

HEADERS	+= ting1_plugin.h \
    nvinterface.h
SOURCES	+= ting1_plugin.cpp \
    nvinterface.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(neutube)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_tracing/tracing_tubes
