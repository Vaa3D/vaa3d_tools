
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../vaa3d/vaa3d_tools/v3d_main
INCLUDEPATH += $$VAA3DPATH/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/common_lib/include

win32 {
    LIBS += -L../../vaa3d/v3d_external/v3d_main/common_lib/winlib64 -llibtiff
}

macx{
    LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
}

unix:!macx {
    LIBS += -L$$V3DMAINPATH/common_lib/lib -ltiff

}

HEADERS	+= Branch_c_plugin.h \
    rayshooting_func.h
SOURCES	+= Branch_c_plugin.cpp \
    rayshooting_func.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= ../../vaa3d/vaa3d_tools/released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp

TARGET	= $$qtLibraryTarget(Branch_c)
DESTDIR	= ../plugins/Branch_c/
