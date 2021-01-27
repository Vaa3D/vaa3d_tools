
TEMPLATE      = lib
CONFIG       += plugin 
#CONFIG       += x86_64
CONFIG += qt warn_off
CONFIG -= app_bundle

V3DMAINPATH  = ../../../v3d_main
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH  += $$V3DMAINPATH/common_lib/include
INCLUDEPATH  += $$V3DMAINPATH/v3d
INCLUDEPATH  += $$V3DMAINPATH/jba/newmat11

unix {
    LIBS         += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
     }


win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS         += -L$$V3DMAINPATH/common_lib/winlib64 -llibnewmat
    } else {
    LIBS         += -L$$V3DMAINPATH/common_lib/winlib -llibnewmat
    }
}

HEADERS      += $$V3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS      += q_warp_affine_tps.h
HEADERS      += q_warp_affine_tps_dialogs.h
HEADERS      += plugin_warp_affine_tps.h

SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES      += q_warp_affine_tps.cpp
SOURCES      += q_warp_affine_tps_dialogs.cpp
SOURCES      += plugin_warp_affine_tps.cpp

TARGET        = $$qtLibraryTarget(plugin_warp_affin_tps)

DESTDIR       = $$V3DMAINPATH/../bin/plugins/image_registration/alignment_affine_TPS_warp  #win32 qmake couldn't handle space in path
#DESTDIR       = $$V3DMAINPATH/../bin/plugins/neuron_utilities/alignment_affine_TPS_warp  #win32 qmake couldn't handle space in path #by Lei @20120722

