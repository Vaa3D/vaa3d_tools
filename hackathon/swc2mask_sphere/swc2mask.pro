TEMPLATE	= lib
CONFIG	+= debug qt plugin warn_off
#CONFIG	+= x86_64

V3DMAINPATH = ../../v3d_main
INCLUDEPATH += $$V3DMAINPATH/basic_c_fun 
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/jba/newmat11

DEFINES += __V3D_CONVERT_PLUGIN__

DEFINES += __USE_CURL_DOWNLOAD__
LIBS += -lcurl
macx{
	LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
	LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
	CONFIG += x86_64
}

win32{
}

unix:!macx {
	#LIBS += -L$$V3DMAINPATH/common_lib/lib -lv3dtiff
	LIBS += -L$$V3DMAINPATH/common_lib/lib -ltiff
	LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
}

HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h
HEADERS += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS += $$V3DMAINPATH/basic_c_fun/stackutil.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_utilities.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_image_lib.h


SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_memory.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp

TARGET	= $$qtLibraryTarget(swc2mask)

#DESTDIR = .
OBJECTS_DIR = bin/plugin/

# =================================================================
HEADERS += basic_parser.h
HEADERS += advanced_parser.h
HEADERS += simple_c.h
HEADERS += common_dialog.h
HEADERS += my_surf_objs.h

SOURCES += simple_c.cpp
SOURCES += my_surf_objs.cpp
# ================================================================

HEADERS += swc2mask_plugin.h
HEADERS += run_main.h
HEADERS += swc2mask.h

SOURCES += swc2mask_plugin.cpp
SOURCES += run_main.cpp
