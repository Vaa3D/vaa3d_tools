#LAST CHANGE BY HANCHUAN PENG. 2011-08-27. PORTING TO V3D PLUGIN FOLDER


TEMPLATE      = lib
CONFIG       += plugin 
#CONFIG       += x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/jba/newmat11
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

unix {
#	LIBS += -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
	LIBS += -L$$V3DMAINDIR/jba/c++ -lv3dnewmat
}

win32 {
#	LIBS 	+= -L$$V3DMAINDIR/common_lib/winlib -llibtiff
        LIBS += -L$$V3DMAINDIR/common_lib/winlib64 -llibnewmat
}

FORMS         = main_src/paradialog_stranno.ui

HEADERS      += $$V3DMAINDIR/basic_c_fun/v3d_message.h
#HEADERS	     += $$V3DMAINDIR/basic_c_fun/stackutil.h
#HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
#HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.h
HEADERS      += $$V3DMAINDIR/jba/c++/convert_type2uint8.h
HEADERS      += $$V3DMAINDIR/worm_straighten_c/spline_cubic.h
HEADERS      += ../celegans_straighten/q_morphology.h
HEADERS      += ../celegans_straighten/q_principalskeleton_detection.h
HEADERS      += ../celegans_straighten/q_celegans_straighten.h
HEADERS      += main_src/q_atlasguided_seganno.h
HEADERS      += main_src/q_paradialog_stranno.h
HEADERS      += main_src/plugin_atlasguided_stranno_partial.h

SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
#SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
#SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp
#SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINDIR/worm_straighten_c/spline_cubic.cpp
SOURCES      += ../celegans_straighten/q_morphology.cpp
SOURCES      += ../celegans_straighten/q_principalskeleton_detection.cpp
SOURCES      += ../celegans_straighten/q_celegans_straighten.cpp
SOURCES      += main_src/q_imresize.cpp
SOURCES      += main_src/q_atlasguided_seganno.cpp
SOURCES      += main_src/q_paradialog_stranno.cpp
SOURCES      += main_src/plugin_atlasguided_stranno_partial.cpp

TARGET        = $$qtLibraryTarget(plugin_atlasguided_stranno_partial)
DESTDIR       = $$V3DMAINDIR/../bin/plugins/celegans/atlasguided_seganno/

