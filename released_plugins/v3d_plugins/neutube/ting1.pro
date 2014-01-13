
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QT += opengl xml
#CONFIG	+= x86_64
VAA3DPATH = ../../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun neutube/neurolabi/c \
    neutube/neurolabi/c/include neutube/neurolabi/gui \
    neutube/neurolabi/lib/genelib/src neutube/neurolabi/build \
    neutube/neurolabi/gui/ext neutube/neurolabi/gui/ext/QsLog

LIBS += -L$${PWD}/neutube/neurolabi/build -lneutube -L$${PWD}/neutube/neurolabi/c/lib -lneurolabi \
    -lxml2 -ljansson -lGLEW -framework AGL -framework OpenGL -lfftw3 -lfftw3f -ldl \
    -framework AppKit -framework IOKit     -framework OpenGL \
    -framework ApplicationServices -ldl -framework AppKit -framework IOKit \
    -framework OpenGL -framework ApplicationServices -framework CoreFoundation

HEADERS	+= ting1_plugin.h \
    nvinterface.h
SOURCES	+= ting1_plugin.cpp \
    nvinterface.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(ting1)
DESTDIR	= $$VAA3DPATH/bin/plugins/ting1/
