# littlequickwarp
# by Lei Qu
# 2012-07-08

TEMPLATE      = app
CONFIG       += qt 
macx: CONFIG -= app_bundle
CONFIG       += x86_64
#QT          -= gui # Only the core module is used

V3DMAINDIR    = ../../../v3d_main
JBADIR        = ../../../../brainaligner

INCLUDEPATH  += $$V3DMAINDIR/common_lib/include
INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/jba/newmat11
INCLUDEPATH  += $$JBADIR/jba/c++/

LIBS 	     += -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
LIBS         += -L$$V3DMAINDIR/jba/c++ -lv3dnewmat

HEADERS      += $$V3DMAINDIR/basic_c_fun/v3d_message.h
HEADERS      += $$V3DMAINDIR//basic_c_fun/basic_surf_objs.h
HEADERS	     += $$V3DMAINDIR/basic_c_fun/stackutil.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$JBADIR/jba/c++/jba_mainfunc.h
HEADERS      += $$JBADIR/jba/c++/jba_match_landmarks.h
HEADERS      += $$JBADIR/jba/c++/remove_nonaffine_points.cpp
HEADERS      += q_interpolate.h

SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
SOURCES      += $$V3DMAINDIR//basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      += $$JBADIR/jba/c++/jba_mainfunc.cpp
SOURCES      += $$JBADIR/jba/c++/jba_match_landmarks.cpp
SOURCES      += $$JBADIR/jba/c++/remove_nonaffine_points.cpp
SOURCES      += q_interpolate.cpp
SOURCES      += q_imgwarp_tps_quicksmallmemory.cpp
SOURCES      += main_imgwarp_tps_quicksmallmemory.cpp


