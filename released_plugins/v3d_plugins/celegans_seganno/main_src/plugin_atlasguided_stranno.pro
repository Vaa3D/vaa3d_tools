
TEMPLATE      = lib
CONFIG       += plugin 
CONFIG       += x86_64

INCLUDEPATH  += ../../v3d_main/basic_c_fun
INCLUDEPATH  += ../../v3d_main/jba/newmat11
INCLUDEPATH  += ../../v3d_main/common_lib/include

LIBS 	     += -L../../v3d_main/common_lib/lib -lv3dtiff
LIBS         += -L../../v3d_main/jba/c++ -lv3dnewmat

FORMS         = paradialog_stranno.ui

HEADERS      += ../../v3d_main/basic_c_fun/v3d_message.h
HEADERS	     += ../../v3d_main/basic_c_fun/stackutil.h
HEADERS      += ../../v3d_main/basic_c_fun/mg_image_lib.h
HEADERS      += ../../v3d_main/basic_c_fun/mg_utilities.h
HEADERS      += ../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS      += ../../v3d_main/worm_straighten_c/spline_cubic.h
HEADERS      += q_imresize.h
HEADERS      += ../celegans_straighten/q_morphology.h
HEADERS      += ../celegans_straighten/q_principalskeleton_detection.h
HEADERS      += ../celegans_straighten/q_celegans_straighten.h
HEADERS      += q_atlasguided_seganno.h
HEADERS      += q_paradialog_stranno.h
HEADERS      += plugin_atlasguided_stranno.h

SOURCES      += ../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES      += ../../v3d_main/basic_c_fun/stackutil.cpp
SOURCES      += ../../v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES      += ../../v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES      += ../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES      += ../../v3d_main/worm_straighten_c/spline_cubic.cpp
SOURCES      += q_imresize.cpp
SOURCES      += ../celegans_straighten/q_morphology.cpp
SOURCES      += ../celegans_straighten/q_principalskeleton_detection.cpp
SOURCES      += ../celegans_straighten/q_celegans_straighten.cpp
SOURCES      += q_atlasguided_seganno.cpp
SOURCES      += q_paradialog_stranno.cpp
SOURCES      += plugin_atlasguided_stranno.cpp

TARGET        = $$qtLibraryTarget(plugin_atlasguided_stranno)


DESTDIR       = ../../v3d_main/v3d/plugins/atlasguided_stranno/ #win32 qmake couldn't handle space in path

