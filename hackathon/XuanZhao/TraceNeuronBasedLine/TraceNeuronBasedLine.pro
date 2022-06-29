
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include

INCLUDEPATH     += $$VAA3DPATH/v3d_main/jba/newmat11


macx{
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
#    CONFIG += x86_64
}

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibtiff
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat
    } else {
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib -llibtiff
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib -llibnewmat
    }
}

unix:!macx {
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib -ltiff
    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
}

#LIBS += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat

HEADERS	+= TraceNeuronBasedLine_plugin.h \
    op_neurontree.h \
    branchtree.h \
    dataset.h \
    swc2mask.h \
    swc_convert.h

HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h

SOURCES	+= TraceNeuronBasedLine_plugin.cpp \
    op_neurontree.cpp \
    branchtree.cpp \
    dataset.cpp \
    swc2mask.cpp \
    swc_convert.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(TraceNeuronBasedLine)
DESTDIR	= $$VAA3DPATH/bin/plugins/TraceNeuronBasedLine/
