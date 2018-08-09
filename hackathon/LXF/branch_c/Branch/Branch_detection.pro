
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/v3d_main/common_lib/include

win32 {
    LIBS += -L$$V3DMAINPATH/v3d_main/common_lib/winlib64 -llibtiff
}

macx{
    LIBS += -L$$V3DMAINPATH/v3d_main/common_lib/lib_mac64 -lv3dtiff
}

unix:!macx {
    LIBS += -L$$V3DMAINPATH/v3d_main/common_lib/lib -ltiff

}

HEADERS	+= Branch_detection_plugin.h
HEADERS	+= new_ray-shooting.h
HEADERS	+= ..\zhi\APP2_large_scale\my_surf_objs.h


SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= Branch_detection_plugin.cpp
SOURCES	+= new_ray-shooting.cpp
SOURCES	+= ..\zhi\APP2_large_scale\my_surf_objs.cpp

TARGET	= $$qtLibraryTarget(Branch_detection)
DESTDIR	= $$V3DMAINPATH/bin/plugins/Branch_detection/
