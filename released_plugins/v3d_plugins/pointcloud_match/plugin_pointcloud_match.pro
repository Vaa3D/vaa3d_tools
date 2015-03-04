
TEMPLATE      = lib
CONFIG       += plugin warn_off

V3DMAINPATH = ../../../v3d_main
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH  += $$V3DMAINPATH/jba/newmat11


win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib64 -llibnewmat
    } else {
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib -llibnewmat
    }
}

unix {
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}

HEADERS      += $$V3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS      += src/q_pointcloud_match_basic.h
HEADERS      += src/q_pointcloud_match_initialmatch.h
HEADERS      += src/q_pointcloud_match_refinematch_manifold.h
HEADERS      += src/q_pointcloud_match_refinematch_affine.h
HEADERS      += src/q_pointcloud_match.h
HEADERS      += src/q_pointcloud_match_dialogs.h
HEADERS      += src/plugin_pointcloud_match.h

SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
#SOURCES      += $$V3DMAINPATH/basic_c_fun/io_ano_file.cpp
SOURCES      += src/q_pointcloud_match_basic.cpp
SOURCES      += src/q_pointcloud_match_initialmatch.cpp
SOURCES      += src/q_pointcloud_match_refinematch_manifold.cpp
SOURCES      += src/q_pointcloud_match_refinematch_affine.cpp
SOURCES      += src/q_pointcloud_match.cpp
SOURCES      += src/q_pointcloud_match_dialogs.cpp
SOURCES      += src/plugin_pointcloud_match.cpp

TARGET        = $$qtLibraryTarget(plugin_pointcloud_match)

DESTDIR       = $$V3DMAINPATH/../bin/plugins/image_registration/pointcloud_matcher/ 
#win32 qmake couldn't handle space in path

