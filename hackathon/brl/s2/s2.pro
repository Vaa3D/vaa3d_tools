
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

VAA3D_DIR = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3D_DIR/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3D_DIR/v3d_main/common_lib/include
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2


QT           += network

HEADERS  = s2_plugin.h \
    s2plot.h \
    stackAnalyzer.h \
    noteTaker.h \
    targetList.h \
    eventLogger.h \
    tileInfo.h \ 
    s2monitor.h
HEADERS += s2UI.h
HEADERS += s2Controller.h
HEADERS += $$VAA3D_DIR/v3d_main/basic_c_fun/customary_structs/vaa3d_neurontoolbox_para.h
HEADERS += $$VAA3D_DIR/v3d_main/basic_c_fun/basic_surf_objs.h


HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_tree.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/hierarchy_prune.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_dt.h



SOURCES = s2_plugin.cpp \
    s2plot.cpp \
    stackAnalyzer.cpp \
    noteTaker.cpp \
    targetList.cpp \
    eventLogger.cpp \
    tileInfo.cpp \ 
    s2monitor.cpp
SOURCES += s2UI.cpp
SOURCES += s2Controller.cpp
SOURCES += $$VAA3D_DIR/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3D_DIR/v3d_main/basic_c_fun/v3d_message.cpp

SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2_connector.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/swc_convert.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp
SOURCES += $$VAA3D_DIR/v3d_main/basic_c_fun/basic_4dimage_create.cpp

TARGET	= $$qtLibraryTarget(s2)
DESTDIR	= $$VAA3D_DIR/bin/plugins/s2/

FORMS += \
    s2plot.ui

OTHER_FILES += \
    taskList.txt
DEFINES += GIT_CURRENT_SHA1="\\\"$(shell git rev-parse HEAD)\\\""
