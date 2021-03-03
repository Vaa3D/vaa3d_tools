
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d

HEADERS	+= Paint_plugin.h \
    scribblearea.h \
   # mainwindow.h \
   # mydialog.h \
    paint_dialog.h
SOURCES	+= Paint_plugin.cpp \
    scribblearea.cpp \
   # mainwindow.cpp \
   # mydialog.cpp \
    paint_dialog.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(paint)
DESTDIR	= $$V3DMAINPATH/bin/plugins/misc/paint/

RESOURCES += \
    paint.qrc


