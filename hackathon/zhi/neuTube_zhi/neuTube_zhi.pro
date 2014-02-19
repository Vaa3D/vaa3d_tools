
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


LIBS += -L/local1/work/v3d_external/v3d_main/common_lib/src_packages/mylib_tiff -lmylib


#LIBS += -L/$$SRCNEUTUBEPATH/neurolabi/lib/fftw-3.3.2 -lfftw3f

HEADERS	+= neuTube_zhi_plugin.h

SOURCES	+= neuTube_zhi_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

SOURCES += ./lib/genelib/src/image_lib.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_lib.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_attribute.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_neighborhood.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_arrayqueue.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_objlabel.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_imatrix.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_mxutils.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_image_array.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_utilities.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_int_histogram.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_iarray.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_string.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_threshold.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_bwmorph.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_objdetect.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_voxel_graphics.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_stack_sampling.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_u16array.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_u8array.c
SOURCES +=  $$SRCNEUTUBEPATH/neurolabi/c/tz_farray.c

TARGET	= $$qtLibraryTarget(neuTube_zhi)
DESTDIR	= ../../../../v3d_external/bin/plugins/neuTube_zhi/
