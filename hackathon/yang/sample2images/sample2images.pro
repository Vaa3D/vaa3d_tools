TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
QMAKE_CXXFLAGS += -std=c++11

VAA3DMAINPATH = ../../../../v3d_external/v3d_main
VAA3DPLUGINPATH = ../../../released_plugins/v3d_plugins

#
INCLUDEPATH	+= $$VAA3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$VAA3DMAINPATH/neuron_editing
INCLUDEPATH     += $$VAA3DMAINPATH/common_lib/include/boost/graph
INCLUDEPATH     += $$VAA3DMAINPATH/common_lib/include

# basic func
HEADERS += $$VAA3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/basic_4dimage.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/mg_utilities.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/mg_image_lib.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/stackutil.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/imageio_mylib.h
HEADERS += $$VAA3DMAINPATH/neuron_editing/neuron_sim_scores.h
HEADERS	+= $$VAA3DMAINPATH/neuron_editing/v_neuronswc.h

SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/basic_4dimage.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$VAA3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$VAA3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$VAA3DMAINPATH/basic_c_fun/imageio_mylib.cpp
SOURCES += $$VAA3DMAINPATH/basic_c_fun/basic_4dimage_create.cpp
SOURCES	+= $$VAA3DMAINPATH/neuron_editing/neuron_sim_scores.cpp
SOURCES	+= $$VAA3DMAINPATH/neuron_editing/v_neuronswc.cpp
SOURCES	+= $$VAA3DMAINPATH/graph/dijk.cpp

# sample 2 images
HEADERS	+= sample2images_plugin.h
HEADERS	+= sample2images_func.h

SOURCES	+= sample2images_plugin.cpp
SOURCES	+= sample2images_func.cpp

#
macx{
    HEADERS += $$VAA3DMAINPATH//io/v3d_nrrd.h
    SOURCES += $$VAA3DMAINPATH//io/v3d_nrrd.cpp

    QMAKE_CXXFLAGS += -stdlib=libc++ -Wno-c++11-narrowing -mmacosx-version-min=10.7
    LIBS += -L$$VAA3DMAINPATH/common_lib/lib_mac64 -lv3dtiff -lz -lbz2 -lteem
    LIBS += -L$$VAA3DMAINPATH/jba/c++ -lv3dnewmat
    LIBS += -framework OpenCL
    LIBS += -lc++
    LIBS += -L$$VAA3DMAINPATH/common_lib/src_packages/mylib_tiff -lmylib
}

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp -pthread
    LIBS += -fopenmp
    LIBS += -L$$VAA3DMAINPATH/jba/c++ -lv3dnewmat
    LIBS += -L$$VAA3DMAINPATH/common_lib/src_packages/mylib_tiff -lmylib
    LIBS += -lOpenCL
}

TARGET	= $$qtLibraryTarget(neurontree_construct)
DESTDIR	= $$VAA3DMAINPATH/../bin/plugins/image_filters/sample2images/
