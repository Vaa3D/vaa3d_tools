
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/
INCLUDEPATH	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/
INCLUDEPATH	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_connector/
INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/sort_neuron_swc/
INCLUDEPATH     += $$VAA3DPATH/v3d_main/jba/newmat11
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= preprocess_plugin.h \
    neurite_analysis_main.h \
    utilities.h \
    sort_swc_redefined.h \
    axon_func.h \
    connnect_swc_redefined.h \
    split_neuron_main.h \
    qc_main.h \
    preprocess_batch_main.h
HEADERS += pre_processing_main.h
SOURCES	+= preprocess_plugin.cpp \
    neurite_analysis_main.cpp \
    utilities.cpp \
    sort_swc_redefined.cpp \
    axon_func.cpp \
    connect_swc_redefined.cpp \
    split_neuron_main.cpp \
    qc_main.cpp \
    preprocess_batch_main.cpp
SOURCES += pre_processing_main.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/align_axis.cpp
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/pca1.cpp
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/prune_short_branch.cpp
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/resampling.cpp

HEADERS += $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_connector/neuron_connector_func.h
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_connector/neuron_connector_func.cpp

HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    INCLUDEPATH	+= C:/gnuwin32/include
    INCLUDEPATH	+= ./
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat
    } else {
    LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib -llibnewmat
    }
}

unix {
    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
}

TARGET	= $$qtLibraryTarget(preprocess)
DESTDIR	= $$VAA3DPATH/bin/plugins/preprocess/
