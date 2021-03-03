
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external

INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d


HEADERS	+= neutu_autotrace_plugin.h libNeuTu.h
SOURCES	+= neutu_autotrace_plugin.cpp libNeuTu.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(neutu_autotrace)
PLUGIN_DIR = plugins/neuron_tracing/neutu_autotrace
PARAMDIR = bin
BINDIR	= $$V3DMAINPATH/$$PARAMDIR
DESTDIR	= $$BINDIR/$$PLUGIN_DIR

DEFINES += NEUTU_PARAM_DIR=\"\\\"$$PARAMDIR\\\"\"
unix {
DEPLOY_COMMAND = "$$PWD/deploy $$BINDIR"
QMAKE_POST_LINK += $$DEPLOY_COMMAND
}
unix:macx {
DEFINES += _MAC_APPLICATION_
}
