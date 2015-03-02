#last change by Hanchuan Peng, 2011-08-27 for porting this plugin to v3d main release folder

TEMPLATE      = lib
CONFIG       += plugin 
#CONFIG       += x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH += $$V3DMAINDIR/jba/newmat11
INCLUDEPATH += $$V3DMAINDIR/common_lib/include

unix {
#	LIBS += -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
	LIBS += -L$$V3DMAINDIR/jba/c++ -lv3dnewmat
}

win32 {
#	LIBS 	     += -L$$V3DMAINDIR/common_lib/winlib -llibtiff
	LIBS         += -L$$V3DMAINDIR/common_lib/winlib -llibnewmat 
}

FORMS         = paradialog.ui

HEADERS      += $$V3DMAINDIR/basic_c_fun/v3d_message.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.h
#HEADERS	     += $$V3DMAINDIR/basic_c_fun/stackutil.h
#HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
#HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINDIR/jba/c++/convert_type2uint8.h
HEADERS      += $$V3DMAINDIR/worm_straighten_c/spline_cubic.h
HEADERS      += q_morphology.h
HEADERS      += q_principalskeleton_detection.h
HEADERS      += q_celegans_straighten.h
HEADERS      += q_paradialog_straighten.h
HEADERS      += plugin_celegans_straighten.h

SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.cpp
#SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
#SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp
#SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      += $$V3DMAINDIR/worm_straighten_c/spline_cubic.cpp
SOURCES      += ../celegans_seganno/main_src/q_imresize.cpp
SOURCES      += q_morphology.cpp
SOURCES      += q_principalskeleton_detection.cpp
SOURCES      += q_celegans_straighten.cpp
SOURCES      += q_paradialog_straighten.cpp
SOURCES      += plugin_celegans_straighten.cpp

TARGET        = $$qtLibraryTarget(celegans_straighten)

DESTDIR       = $$V3DMAINDIR/../bin/plugins/celegans/celegans_straighten/ #win32 qmake couldn't handle space in path


