TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d
INCLUDEPATH	+= $$V3DMAINPATH/../vaa3d_tools/released_plugins/v3d_plugins/eswc_converter
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/jba/newmat11

macx{
    LIBS += -L$$V3DMAINPATH/v3d_main/jba/c++ -lv3dnewmat
#    CONFIG += x86_64
}

win32{
    LIBS += -L$$V3DMAINPATH/v3d_main/common_lib/winlib64 -llibnewmat
}

unix:!macx {
    LIBS += -L$$V3DMAINPATH/v3d_main/jba/c++ -lv3dnewmat
}


HEADERS	+= neuron_image_profiling_plugin.h \
    compute_tubularity.h
SOURCES	+= neuron_image_profiling_plugin.cpp \
    compute_tubularity.cpp
SOURCES	+= profile_swc.cpp
HEADERS	+= profile_swc.h
SOURCES += openSWCDialog.cpp
HEADERS += openSWCDialog.h
SOURCES += $$V3DMAINPATH/../vaa3d_tools/released_plugins/v3d_plugins/eswc_converter/eswc_core.cpp
HEADERS += $$V3DMAINPATH/../vaa3d_tools/released_plugins/v3d_plugins/eswc_converter/eswc_core.h


SOURCES += $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
HEADERS	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.h





TARGET	= $$qtLibraryTarget(neuron_image_profiling)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_utilities/neuron_image_profiling/
