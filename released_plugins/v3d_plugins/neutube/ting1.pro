
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QT += opengl xml
#CONFIG	+= x86_64
VAA3DPATH = ../../../v3d_main
INCLUDEPATH += $$VAA3DPATH/v3d $$VAA3DPATH/basic_c_fun $$VAA3DPATH/common_lib/include   src_neutube/neurolabi/c \
    src_neutube/neurolabi/c/include src_neutube/neurolabi/gui \
    src_neutube/neurolabi/lib/genelib/src src_neutube/neurolabi/build \
    src_neutube/neurolabi/gui/ext src_neutube/neurolabi/gui/ext/QsLog

INCLUDEPATH += src_neutube/neurolabi/lib/libpng-1.6.7 src_neutube/neurolabi/lib/jansson/include


LIBS += -L$${PWD}/src_neutube/neurolabi/build -lneutube -L$${PWD}/src_neutube/neurolabi/c/lib -lneurolabi \
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
DESTDIR	= ../../v3d/plugins/neuron_tracing/tracing_tubes
