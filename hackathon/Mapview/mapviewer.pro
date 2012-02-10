
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
CONFIG       += x86_64
#CONFIG       +=release

V3DMAINDIR = ../../../vaa3d/v3d_main
QT_DIR = $$dirname(QMAKE_QMAKE)/..

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include
INCLUDEPATH  += $$QT_DIR/demos/shared/
#HEADERS      +=/usr/local/Trolltech/Qt-4.5.2/demos/shared/arthurwidgets.h
HEADERS      +=colormap.h

HEADERS      += mg_image_lib11.h
HEADERS      += stackutil-11.h
#HEADERS      += ../../../vaa3d/released_plugins/v3d_plugins/istitch/y_imglib.h
HEADERS      += $$V3DMAINDIR/../released_plugins/v3d_plugins/istitch/y_imglib.h

HEADERS      += mapviewer.h \
#		$$V3DMAINDIR/basic_c_fun/stackutil.h \
		$$V3DMAINDIR/basic_c_fun/mg_utilities.h \
#		$$V3DMAINDIR/basic_c_fun/mg_image_lib.h 

SOURCES       += mg_image_lib11.cpp 
SOURCES       += stackutil-11.cpp
SOURCES       += mapviewer.cpp \
#		$$V3DMAINDIR/basic_c_fun/stackutil.cpp \
		$$V3DMAINDIR/basic_c_fun/mg_utilities.cpp \
#		$$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp

LIBS     += -lm -lpthread
LIBS	    += -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
        += -L$$QTDIR/demos/shared/
#LIBS += -framework CoreServices
TARGET        = $$qtLibraryTarget(Multi_Level_mapview)
#DESTDIR       = ../../../vaa3d/v3d/plugins/Multi_Level_mapview
DESTDIR       = $$V3DMAINDIR/../v3d/plugins/Multi_Level_mapview

