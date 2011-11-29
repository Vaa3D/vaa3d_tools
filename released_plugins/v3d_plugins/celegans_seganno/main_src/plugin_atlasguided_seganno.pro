
TEMPLATE      = lib
CONFIG       += plugin 
CONFIG       += x86_64

INCLUDEPATH  += ../../v3d_main/basic_c_fun
INCLUDEPATH  += ../../v3d_main/jba/newmat11

LIBS 	     += -L../../v3d_main/common_lib/lib -lv3dtiff
LIBS         += -L../../v3d_main/jba/c++ -lv3dnewmat

FORMS         = paradialog.ui

HEADERS      += ../../v3d_main/basic_c_fun/v3d_message.h
HEADERS	     += ../../v3d_main/basic_c_fun/stackutil.h
HEADERS      += ../../v3d_main/basic_c_fun/mg_image_lib.h
HEADERS      += ../../v3d_main/basic_c_fun/mg_utilities.h
HEADERS      += ../../v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS      += q_imresize.h
HEADERS      += q_atlasguided_seganno.h
HEADERS      += q_paradialog.h
HEADERS      += plugin_atlasguided_seganno.h

SOURCES      += ../../v3d_main/basic_c_fun/v3d_message.cpp
SOURCES      += ../../v3d_main/basic_c_fun/stackutil.cpp
SOURCES      += ../../v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES      += ../../v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES      += ../../v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES      += q_imresize.cpp
SOURCES      += q_atlasguided_seganno.cpp
SOURCES      += q_paradialog.cpp
SOURCES      += plugin_atlasguided_seganno.cpp

TARGET        = $$qtLibraryTarget(plugin_atlasguided_seganno)


DESTDIR       = ../../v3d_main/v3d/plugins/atlasguided_seganno/ #win32 qmake couldn't handle space in path

