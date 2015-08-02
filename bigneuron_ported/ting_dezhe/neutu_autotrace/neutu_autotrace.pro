
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9

#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external

INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include


HEADERS	+= neutu_autotrace_plugin.h libNeuTu.h
SOURCES	+= neutu_autotrace_plugin.cpp libNeuTu.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(neutu_autotrace)
PLUGIN_DIR = plugins/neuron_tracing/neutu_autotrace
DESTDIR	= $$VAA3DPATH/bin/$$PLUGIN_DIR

DEFINES += PLUGIN_DIR=\"\\\"$$PLUGIN_DIR\\\"\"
unix {
DEPLOY_COMMAND = "$$PWD/deploy $$DESTDIR"
QMAKE_POST_LINK += $$DEPLOY_COMMAND
}
unix:macx {
DEFINES += _MAC_APPLICATION_
}
