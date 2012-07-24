# littlequickwarp
# by Lei Qu
# 2012-07-16

TEMPLATE	 = lib
CONFIG		+= qt plugin warn_off
CONFIG		+= x86_64

V3DMAINPATH   = ../../../v3d_main
JBADIR        = ../../../../brainaligner

INCLUDEPATH  += $$V3DMAINPATH/common_lib/include
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH  += $$V3DMAINPATH/jba/newmat11
INCLUDEPATH  += $$JBADIR/jba/c++/

LIBS 	     += -L$$V3DMAINPATH/common_lib/lib -lv3dtiff
LIBS         += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat

FORMS         = q_paradialog_littlequickwarp.ui

HEADERS      += $$V3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS      += $$V3DMAINPATH//basic_c_fun/basic_surf_objs.h
HEADERS	     += $$V3DMAINPATH/basic_c_fun/stackutil.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/mg_utilities.h
HEADERS      += $$JBADIR/jba/c++/jba_mainfunc.h
HEADERS      += $$JBADIR/jba/c++/jba_match_landmarks.h
HEADERS      += $$JBADIR/jba/c++/remove_nonaffine_points.cpp
HEADERS      += q_paradialog_littlequickwarp.h
HEADERS      += q_interpolate.h
HEADERS      += q_bspline.h
HEADERS      += plugin_littlequickwarp.h

SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES      += $$V3DMAINPATH//basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES      += $$JBADIR/jba/c++/jba_mainfunc.cpp
SOURCES      += $$JBADIR/jba/c++/jba_match_landmarks.cpp
SOURCES      += $$JBADIR/jba/c++/remove_nonaffine_points.cpp
SOURCES      += q_paradialog_littlequickwarp.cpp
SOURCES      += q_interpolate.cpp
SOURCES      += q_bspline.cpp
SOURCES      += q_imgwarp_tps_quicksmallmemory.cpp
SOURCES      += plugin_littlequickwarp.cpp


TARGET	= $$qtLibraryTarget(littlequickwarp)
#DESTDIR	= ../../v3d/plugins/littlequickwarp/
DESTDIR	= ../../../bin/plugins/image_registration/littleQuickWarp/
