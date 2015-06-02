
TEMPLATE = lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../..
V3DMAINPATH = $$VAA3DPATH/v3d_main
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/jba/newmat11
INCLUDEPATH += $$V3DMAINPATH/jba/c++

HEADERS	+= jba_plugin.h
HEADERS += $$V3DMAINPATH/jba/c++/convert_type2uint8.h
HEADERS += $$V3DMAINPATH/jba/c++/jba_match_landmarks.h
HEADERS += $$V3DMAINPATH/jba/c++/displacefield_comput.h
HEADERS += $$V3DMAINPATH/jba/c++/remove_nonaffine_points.h
HEADERS += $$V3DMAINPATH/jba/c++/histeq.h
HEADERS += $$V3DMAINPATH/jba/c++/seg_fly_brain.h
HEADERS += $$V3DMAINPATH/jba/c++/jba_affine_xform.h
HEADERS += $$V3DMAINPATH/jba/c++/wkernel.h
HEADERS += $$V3DMAINPATH/jba/c++/jba_mainfunc.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_utilities.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_image_lib.h
unix:HEADERS += $$V3DMAINPATH/basic_c_fun/imageio_mylib.h

SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/jba/c++/histeq.cpp
SOURCES	+= $$V3DMAINPATH/jba/c++/convert_type2uint8.cpp
SOURCES	+= $$V3DMAINPATH/jba/c++/wkernel.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
unix:SOURCES += $$V3DMAINPATH/basic_c_fun/imageio_mylib.cpp
SOURCES	+= basic_volproc3d.cpp
SOURCES	+= jba_mainfunc.cpp
SOURCES	+= jba_affine_xform.cpp
SOURCES	+= jba_match_landmarks.cpp
SOURCES	+= remove_nonaffine_points.cpp
SOURCES	+= jba_plugin.cpp

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib64 -llibnewmat
    } else {
    LIBS     += -L$$V3DMAINPATH/common_lib/winlib -llibnewmat
    }
}

unix {
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
    LIBS += -L$$V3DMAINPATH/common_lib/lib -lv3dtiff
    LIBS += -L$$V3DMAINPATH/common_lib/src_packages/mylib_tiff -lmylib
}

TARGET	= $$qtLibraryTarget(brainaligner)
DESTDIR	= $$VAA3DPATH/bin/plugins/image_registration/brainaligner/

