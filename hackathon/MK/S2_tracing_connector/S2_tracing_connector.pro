
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= S2_tracing_connector_plugin.h \
            S2_tracing_connector_func.h \
            ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h

SOURCES	+= S2_tracing_connector_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp \
            $$INCLUDEPATH/basic_surf_objs.cpp \
            ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp \
            S2_tracing_connector_func.cpp

TARGET	= $$qtLibraryTarget(S2_tracing_connector)
DESTDIR	= $$VAA3DPATH/bin/plugins/S2_tracing_connector/
