
TEMPLATE      = app
CONFIG       += qt
macx: CONFIG         -= app_bundle
#CONFIG       += x86_64 #this cannot be added as there will be 32bit compatibility issue. Should add on command line. by PHC, 101223
#QT          -= gui # Only the core module is used

V3DMAINDIR = ../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/jba/newmat11

LIBS 	     += -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
LIBS         += -L$$V3DMAINDIR/jba/c++ -lv3dnewmat

HEADERS      += $$V3DMAINDIR/basic_c_fun/v3d_message.h
HEADERS	     += $$V3DMAINDIR/basic_c_fun/stackutil.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.h
HEADERS      += $$V3DMAINDIR/jba/c++/convert_type2uint8.h
HEADERS      += $$V3DMAINDIR/worm_straighten_c/spline_cubic.h
HEADERS      += ../celegans_straighten/q_morphology.h
HEADERS      += ../celegans_straighten/q_principalskeleton_detection.h
HEADERS      += ../celegans_straighten/q_celegans_straighten.h
HEADERS      += q_atlasguided_seganno.h

SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINDIR/worm_straighten_c/spline_cubic.cpp
SOURCES      += ../celegans_straighten/q_morphology.cpp
SOURCES      += ../celegans_straighten/q_principalskeleton_detection.cpp
SOURCES      += ../celegans_straighten/q_celegans_straighten.cpp
SOURCES      += ../common/q_imresize.cpp
SOURCES      += q_atlasguided_seganno.cpp
SOURCES      += main_atlasguided_stranno.cpp


