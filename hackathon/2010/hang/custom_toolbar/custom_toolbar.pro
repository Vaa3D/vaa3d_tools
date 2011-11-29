
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
INCLUDEPATH	+= ../../../v3d_main/basic_c_fun
INCLUDEPATH	+= ../../../v3d_main/custom_toolbar

DEFINES += __v3d_custom_toolbar_plugin__

HEADERS	= custom_toolbar_plugin.h
HEADERS	+= custom_toolbar_func.h
HEADERS	+= ../../../v3d_main/custom_toolbar/v3d_custom_toolbar.h

SOURCES	= custom_toolbar_plugin.cpp
SOURCES	+= custom_toolbar_func.cpp
SOURCES += ../../../v3d_main/custom_toolbar/v3d_custom_toolbar.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/v3d_message.cpp

RESOURCES += custom_toolbar.qrc

TARGET	= $$qtLibraryTarget(custom_toolbar)
DESTDIR	= ~/Applications/v3d/plugins/custom_toolbar/

