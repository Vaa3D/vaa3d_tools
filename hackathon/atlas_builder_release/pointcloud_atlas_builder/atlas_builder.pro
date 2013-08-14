# #####################################################################
# Fuhui Long
# 090520
# ######################################################################

TEMPLATE = app
TARGET += 
DEPENDPATH += .
INCLUDEPATH += .

QT_DIR = $$dirname(QMAKE_QMAKE)/.. 

LOCAL_DIR = /usr/local # platform: macx, unix, win32-msys-mingw
WINGW_DIR = /mingw # platform: unix, win32-msys-mingw
win32 { # platform: win32-mingw
	WINGW_DIR = c:/mingw
	LOCAL_DIR = c:/msys/local
	CONFIG = $$unique(CONFIG)
	CONFIG -= debug # for Qt-win32 which only has release install(no debug)
}
win32:LOCAL_DIR = ../v3d_main/common_lib # copy from /usr/local 

SHARED_FOLDER = $$QT_DIR/demos/shared # for arthurwidgets
include($$SHARED_FOLDER/shared.pri)
INCLUDEPATH += $$SHARED_FOLDER
LIBS += -L$$SHARED_FOLDER \
	-L$$SHARED_FOLDER/release # for Qt-win32 which only has release install(no debug)

INCLUDEPATH += $$LOCAL_DIR/include 
LIBS += -L$$LOCAL_DIR/lib \
	-L$$WINGW_DIR/lib

# the following trick was figured out by Ruan Zongcai
# CONFIG += release ### disable most of warnings
CONFIG += warn_off # ## only work for complier

# Gene's suggestion to remove annoying QT ld warnings
# QMAKE_CXXFLAGS += -fvisibility=hidden # unrecognized command line option on mingw
# Input
HEADERS += \
	../../../../v3d_external/v3d_main/basic_c_fun/color_xyz.h \
	pointcloud_atlas_io.h \
	../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.h \
	../../../../v3d_external/v3d_main/basic_c_fun/v3d_message.h \
	FL_atlasBuilder.h \
	converter_pcatlas_data.h \
	FL_registerAffine.h \
	../../../../v3d_external/v3d_main/cellseg/FL_sort2.h 

SOURCES += \
	pointcloud_atlas_io.cpp \
	../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp \
	../../../../v3d_external/v3d_main/basic_c_fun/v3d_message.cpp \
	pbetai.cpp \
	FL_atlasBuilder.cpp \
	converter_pcatlas_data.cpp \ 
	FL_registerAffine.cpp \
	FL_main_atlasBuilder.cpp 

#FORMS += 

#RESOURCES += v3d.qrc
#RESOURCES += ../3drenderer/3drenderer.qrc
QT += opengl

macx:LIBS += -lm \
    -L../../../../v3d_external/v3d_main/common_lib/lib/ -lv3dtiff \
    -L../../../../v3d_external/v3d_main/jba/c++ \
    -lv3dnewmat 

win32:LIBS += \
    -L../../../../v3d_external/v3d_main/common_lib/lib/ -lv3dtiff \
    -L../../../../v3d_external/v3d_main/jba/c++ \
    -lv3dnewmat 

unix:LIBS += \
    -L../../../../v3d_external/v3d_main/common_lib/lib/ -lv3dtiff \
    -L../../../../v3d_external/v3d_main/jba/c++ \
    -lv3dnewmat 

INCLUDEPATH = $$unique(INCLUDEPATH)
LIBS = $$unique(LIBS)
# ##CONFIG = $$unique(CONFIG) # this only NOT works on macx, very strange, by RZC 20080923
