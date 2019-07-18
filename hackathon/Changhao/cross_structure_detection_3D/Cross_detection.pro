
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include

win32 {
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibtiff
}

macx{
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib_mac64 -lv3dtiff
}

unix:!macx {
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib -ltiff

}

HEADERS	+= Cross_detection_plugin.h
HEADERS	+= ClusterAnalysis_20190518.h
HEADERS	+= new_ray-shooting.h
HEADERS	+= branch_detection_model.h
HEADERS	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/pruning_swc/my_surf_objs.h
HEADERS	+= neuron_tools.h
SOURCES	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/pruning_swc/my_surf_objs.cpp
SOURCES	+= Cross_detection_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= new_ray-shooting.cpp
SOURCES	+= branch_detection_model.cpp
SOURCES	+= ClusterAnalysis_20190518.cpp
SOURCES	+= neuron_tools.cpp

TARGET	= $$qtLibraryTarget(Cross_detection)
DESTDIR	= $$VAA3DPATH/bin/plugins/Guo/Cross_detection_3D/
