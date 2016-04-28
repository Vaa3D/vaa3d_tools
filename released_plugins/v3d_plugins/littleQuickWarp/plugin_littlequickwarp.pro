# littlequickwarp
# by Lei Qu
# 2012-07-16

TEMPLATE	 = lib
CONFIG		+= qt plugin warn_off
#CONFIG		+= x86_64

V3DMAINPATH   = ../../../v3d_main
JBADIR        = ../jba

INCLUDEPATH  += $$V3DMAINPATH/common_lib/include
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH  += $$V3DMAINPATH/jba/newmat11
INCLUDEPATH  += $$JBADIR/
INCLUDEPATH  += $$V3DMAINPATH/jba/c++

macx{
    LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib64 -llibtiff
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib64 -llibnewmat
    } else {
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib -llibtiff
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib -llibnewmat
    }
}

unix:!macx {
    LIBS += -L$$V3DMAINPATH/common_lib/lib -lv3dtiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}

FORMS         = q_paradialog_littlequickwarp.ui

HEADERS      += $$V3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS      += $$V3DMAINPATH//basic_c_fun/basic_surf_objs.h
HEADERS	     += $$V3DMAINPATH/basic_c_fun/stackutil.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINPATH/jba/c++/jba_mainfunc.h
HEADERS      += $$V3DMAINPATH/jba/c++/jba_match_landmarks.h
HEADERS      += $$JBADIR/remove_nonaffine_points.cpp
HEADERS      += q_paradialog_littlequickwarp.h
HEADERS      += q_interpolate.h
HEADERS      += q_bspline.h
HEADERS      += plugin_littlequickwarp.h

SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES      += $$V3DMAINPATH//basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES      += $$JBADIR/jba_mainfunc.cpp
SOURCES      += $$JBADIR/jba_match_landmarks.cpp
SOURCES      += $$JBADIR/remove_nonaffine_points.cpp
SOURCES      += q_paradialog_littlequickwarp.cpp
SOURCES      += q_interpolate.cpp
SOURCES      += q_bspline.cpp
SOURCES      += q_imgwarp_tps_quicksmallmemory.cpp
SOURCES      += plugin_littlequickwarp.cpp


TARGET	= $$qtLibraryTarget(littlequickwarp)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/image_registration/littleQuickWarp/
