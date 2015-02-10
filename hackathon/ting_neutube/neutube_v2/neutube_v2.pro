
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
neutube = ../NeuTu

INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$neutube/neurolabi/gui
INCLUDEPATH	+= $$neutube/neurolabi/c
INCLUDEPATH	+= $$neutube/neurolabi/c/include
INCLUDEPATH	+= $$neutube/neurolabi/lib/genelib/src
INCLUDEPATH	+= $$neutube/neurolabi/lib/jansson/include


HEADERS	+= neutube_v2_plugin.h
SOURCES	+= neutube_v2_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

LIBS    += -L$$neutube/neurolabi/cpp/lib/build -lneutube\
           -L$$neutube/neurolabi/c/lib -lneurolabi \
           -L$$neutube/neurolabi/lib/fftw3/lib\
           -L$$neutube/neurolabi/lib/jansson/lib\
           -L$$neutube/neurolabi/lib/xml/lib\
            -L$$neutube/neurolabi/lib/hdf5/lib -ljansson -lxml2 -lfftw3f -lfftw3 -lm -lz -lhdf5

TARGET	= $$qtLibraryTarget(neutube_v2)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_tracing/neutube_v2/
