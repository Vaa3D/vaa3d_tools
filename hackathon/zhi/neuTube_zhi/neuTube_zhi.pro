
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QT += opengl xml
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
SRCNEUTUBEPATH = ../../../released_plugins/v3d_plugins/neutube/src_neutube

INCLUDEPATH += $$VAA3DPATH/v3d $$VAA3DPATH/basic_c_fun $$VAA3DPATH/common_lib/include   $$SRCNEUTUBEPATH/neurolabi/c \
    $$SRCNEUTUBEPATH/neurolabi/c/include $$SRCNEUTUBEPATH/neurolabi/gui \
    $$SRCNEUTUBEPATH/neurolabi/lib/genelib/src $$SRCNEUTUBEPATH/neurolabi/build \
    $$SRCNEUTUBEPATH/neurolabi/gui/ext $$SRCNEUTUBEPATH/neurolabi/gui/ext/QsLog

INCLUDEPATH += $$SRCNEUTUBEPATH/neurolabi/lib/libpng-1.6.7 $$SRCNEUTUBEPATH/neurolabi/lib/jansson/include


#LIBS += -L$${PWD}/$$SRCNEUTUBEPATH/neurolabi/build -lneutube -L$${PWD}/$$SRCNEUTUBEPATH/neurolabi/c/lib -lneurolabi \
#    -lxml2 -ljansson -lGLEW -framework AGL -framework OpenGL -lfftw3 -lfftw3f -ldl \
#    -framework AppKit -framework IOKit     -framework OpenGL \
#    -framework ApplicationServices -ldl -framework AppKit -framework IOKit \
#    -framework OpenGL -framework ApplicationServices -framework CoreFoundation


HEADERS	+= neuTube_zhi_plugin.h\
    nvinterface.h

SOURCES	+= neuTube_zhi_plugin.cpp\
    nvinterface.cpp

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

#SOURCES += /home/zhi/Dropbox/neuTube/neurolabi/lib/genelib/src/image_lib.c
#SOURCES += /home/zhi/work/vaa3d_tools/released_plugins/v3d_plugins/neutube/src_neutube/neurolabi/gui/mylib/utilities.cpp

TARGET	= $$qtLibraryTarget(neuTube_zhi)
DESTDIR	= ../../../../v3d_external/bin/plugins/neuTube_zhi/
