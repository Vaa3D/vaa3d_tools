
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
QMAKE_CFLAGS = -Zc:wchar_t
#CONFIG	+= x86_64

VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += ../../../released_plugins/v3d_plugins/swc2mask_cylinder
INCLUDEPATH += ../v3d_imgManagerMK/imgProcessor
INCLUDEPATH += ../v3d_imgManagerMK

win32 {
    BOOSTPATH = $$(BOOST_PATH)
    MSVCVERSION = $$(QMAKESPEC)

    INCLUDEPATH += $$BOOSTPATH
    LIBS += -L../v3d_imgManagerMK
    LIBS += -lv3d_imgManagerMK

    equals(MSVCVERSION, "win32-msvc2013") {
		LIBS += -L$$BOOSTPATH/lib64-msvc-12.0
	}
	equals(MSVCVERSION, "win32-msvc2010") {
		LIBS += -L$$BOOSTPATH/lib64-msvc-10.0
	}
}

unix:!macx {
    INCLUDEPATH += $VAA3DPATH/v3d_main/common_lib/include

    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib
}


HEADERS	+= IVSCC_Auto_Reconstruction_plugin.h
HEADERS += SegPipe_Controller.h

SOURCES	+= IVSCC_Auto_Reconstruction_plugin.cpp
SOURCES += SegPipe_Controller.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(IVSCC_Auto_Reconstruction)
DESTDIR	= $$VAA3DPATH/bin/plugins/IVSCC_Auto_Reconstruction/
