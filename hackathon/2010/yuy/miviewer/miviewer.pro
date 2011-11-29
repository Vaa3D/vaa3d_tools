
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
CONFIG       += x86_64

INCLUDEPATH  += ../../../v3d_main/basic_c_fun
INCLUDEPATH  += ../../../v3d_main/common_lib/include

HEADERS       = miviewer.h \
		../../../v3d_main/basic_c_fun/stackutil.h \
		../../../v3d_main/basic_c_fun/mg_utilities.h \
		../../../v3d_main/basic_c_fun/mg_image_lib.h

SOURCES       += miviewer.cpp \
		../../../v3d_main/basic_c_fun/stackutil.cpp \
		../../../v3d_main/basic_c_fun/mg_utilities.cpp \
		../../../v3d_main/basic_c_fun/mg_image_lib.cpp

LIBS         += -lm -lpthread
LIBS	     += -L../../../v3d_main/common_lib/lib -lv3dtiff

TARGET        = $$qtLibraryTarget(miviewer)
DESTDIR       = ../../../v3d/plugins/miviewer

