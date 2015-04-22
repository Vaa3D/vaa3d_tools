
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
V3DTOOLPATH = ../../..
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/jba/newmat11

TARGET	= $$qtLibraryTarget(smartTrace)
DESTDIR	= $$VAA3DPATH/bin/plugins/bigneuronhackathon/smartTrace/

macx{
    LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
    CONFIG += x86_64
}

win32{
}

unix:!macx {
    #LIBS += -L$$V3DMAINPATH/common_lib/lib -lv3dtiff
    LIBS += -L$$V3DMAINPATH/common_lib/lib -ltiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}


#===============src=================================================
HEADERS += src/nt_selfcorrect_func.h
HEADERS	+= smartTrace_plugin.h

SOURCES	+= smartTrace_plugin.cpp
SOURCES += src/nt_selfcorrect_func.cpp


#===============vaa3d===============================================
HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_utilities.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_image_lib.h
HEADERS += $$V3DMAINPATH/basic_c_fun/stackutil.h

SOURCES += $$V3DMAINPATH/basic_c_fun/basic_memory.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
#SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

#===============mRMR================================================
HEADERS += mrmr/gnu_getline.h \
    mrmr/nrutil.h \
    mrmr/mrmr.h

SOURCES += mrmr/sort2.cpp \
    mrmr/crank.cpp \
    mrmr/gnu_getline.c \
    mrmr/mrmr.cpp \
    mrmr/nrutil.cpp \
    mrmr/pbetai.cpp

#===============SVM=================================================
SOURCES += svm/svm.cpp

HEADERS += svm/svm.h

#===============Hang topology analysis==============================
HEADERS += hang/common_macro.h
HEADERS += hang/fastmarching_linker.h
HEADERS += hang/fastmarching_tree.h
HEADERS += hang/heap.h
HEADERS += hang/simple_c.h
HEADERS += hang/topology_analysis.h
HEADERS += hang/my_surf_objs.h

SOURCES += hang/simple_c.cpp
SOURCES += hang/my_surf_objs.cpp

#===============Radius Estimation===================================
INCLUDEPATH += $$V3DTOOLPATH/released_plugins/v3d_plugins/neurontracing_vn2/neuron_radius
HEADERS += $$V3DTOOLPATH/released_plugins/v3d_plugins/neurontracing_vn2/neuron_radius/marker_radius.h
