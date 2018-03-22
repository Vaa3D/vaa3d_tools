
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$V3DMAINPATH/v3d_main/common_lib/include
#LIBS += -L. -lv3dtiff -L$$V3DMAINPATH/v3d_main/common_lib/winlib
#LIBS         += -L$$V3DMAINPATH/v3d_main/common_lib/lib -lv3dtiff
win32 {
    LIBS += -L$$V3DMAINPATH/v3d_main/common_lib/winlib64 -llibtiff
}

macx{
    LIBS += -L$$V3DMAINPATH/v3d_main/common_lib/lib_mac64 -lv3dtiff
}

unix:!macx {
    LIBS += -L$$V3DMAINPATH/v3d_main/common_lib/lib -ltiff

}

HEADERS	+= Tip_Detection_plugin.h
HEADERS	+= ray_shooting.h
SOURCES	+= Tip_Detection_plugin.cpp
SOURCES	+= ray_shooting.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(Tip_Detection)
DESTDIR	= $$V3DMAINPATH/bin/plugins/Tip_Detection/
