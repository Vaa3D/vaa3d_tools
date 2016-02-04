
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += x86_64
#CONFIG       +=release

#include(/usr/local/Trolltech/Qt-4.5.2/demos/shared/shared.pri)

V3DMAINDIR = ../../../v3d_main
QT_DIR = $$dirname(QMAKE_QMAKE)/..

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include
INCLUDEPATH  += ../istitch
INCLUDEPATH  += $$QT_DIR/demos/shared/
#HEADERS      +=/usr/local/Trolltech/Qt-4.5.2/demos/shared/arthurwidgets.h
HEADERS      +=colormap.h

HEADERS      += mg_image_lib11.h
HEADERS      += stackutil-11.h
HEADERS      += ../istitch/y_imglib.h


HEADERS      += mapviewer.h \
		$$V3DMAINDIR/basic_c_fun/mg_utilities.h 

SOURCES       += mg_image_lib11.cpp 
SOURCES       += stackutil-11.cpp
SOURCES       += mapviewer.cpp \
		$$V3DMAINDIR/basic_c_fun/mg_utilities.cpp 
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp

win32 {
    contains(QMAKE_HOST.arch, x86_64) {
    LIBS     += -L$$V3DMAINDIR/common_lib/winlib64 -llibtiff
    } else {
    LIBS     += -L$$V3DMAINDIR/common_lib/winlib -llibtiff
    }
}

unix {
    LIBS     += -lm -lpthread
    LIBS     += -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
}


LIBS      += -L$$QTDIR/demos/shared/
#LIBS += -framework CoreServices
TARGET        = $$qtLibraryTarget(mapviewer)
DESTDIR       = $$V3DMAINDIR/../bin/plugins/image_stitching/Map_View

