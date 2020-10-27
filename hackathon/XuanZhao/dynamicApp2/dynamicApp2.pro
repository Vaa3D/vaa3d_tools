
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH     += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/common_lib/include
INCLUDEPATH     += app2

#unix {
#LIBS += -L$$V3DMAINPATH/jba/c++
#LIBS += -lv3dnewmat
#}
#win32 {
#LIBS += -L$$V3DMAINPATH/common_lib/winlib64
#LIBS += -llibnewmat
#}

DEFINES += QT_MESSAGELOGCONTEXT

HEADERS	+= dynamicApp2_plugin.h \
    axontrace.h \
    dlog.h
HEADERS += vn_imgpreprocess.h
HEADERS += vn.h
HEADERS += vn_app2.h
HEADERS += app2/fastmarching_tree.h
HEADERS += app2/hierarchy_prune.h
HEADERS += app2/fastmarching_dt.h
HEADERS += app2/my_surf_objs.h
HEADERS += app2/marker_radius.h
HEADERS += swc_convert.h

HEADERS += $$V3DMAINPATH/../../vaa3d_tools/released_plugins/v3d_plugins/swc2mask_cylinder/src/swc2mask.h

HEADERS += $$V3DMAINPATH/../../vaa3d_tools/released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h


SOURCES	+= dynamicApp2_plugin.cpp \
    axontrace.cpp
SOURCES += vn_imgpreprocess.cpp
SOURCES += app2_connector.cpp
SOURCES += swc_convert.cpp
SOURCES += app2/my_surf_objs.cpp

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_4dimage_create.cpp

TARGET	= $$qtLibraryTarget(dynamicApp2)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/dynamicApp2/
