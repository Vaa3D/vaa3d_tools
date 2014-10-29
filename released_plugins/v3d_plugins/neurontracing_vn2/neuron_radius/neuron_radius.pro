
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include

macx {
LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib_mac64 -lv3dtiff
}

unix:!macx {
LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib -lv3dtiff
}


HEADERS	+= neuron_radius_plugin.h
HEADERS	+= my_surf_objs.h
HEADERS	+= marker_radius.h 
SOURCES	+= neuron_radius_plugin.cpp
SOURCES += my_surf_objs.cpp

SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/stackutil.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp


TARGET	= $$qtLibraryTarget(neuron_radius)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_radius/
