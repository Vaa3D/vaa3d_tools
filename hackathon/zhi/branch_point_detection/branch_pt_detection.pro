
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH =../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

unix:!macx {
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat
}


HEADERS	+= branch_pt_detection_plugin.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_smarttracing/hang/my_surf_objs.h

SOURCES	+= branch_pt_detection_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_smarttracing/hang/my_surf_objs.cpp

TARGET	= $$qtLibraryTarget(branch_pt_detection)
DESTDIR	= $$VAA3DPATH/../bin/plugins/branch_pt_detection/
