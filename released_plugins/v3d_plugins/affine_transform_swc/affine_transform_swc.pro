TEMPLATE	= lib
CONFIG	+= qt plugin warn_off

V3DMAINPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d
INCLUDEPATH  += $$V3DMAINPATH/jba/newmat11

macx{
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
#    CONFIG += x86_64
}

win32{
    LIBS += -L$$V3DMAINPATH/common_lib/winlib64 -llibnewmat
}

unix:!macx {
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}


HEADERS	+= affine_transform_swc_plugin.h
HEADERS	+= apply_transform_func.h
HEADERS	+= $$V3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS += io_affine_transform.h

SOURCES	+= affine_transform_swc_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= apply_transform_func.cpp
SOURCES	+= io_affine_transform.cpp


TARGET	= $$qtLibraryTarget(affine_transform_swc)
DESTDIR	= $$V3DMAINPATH/bin/plugins/neuron_utilities/affine_transform_swc/
