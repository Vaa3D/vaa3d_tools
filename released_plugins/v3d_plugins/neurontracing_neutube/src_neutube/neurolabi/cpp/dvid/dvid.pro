TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
QT += webkit network

NEUROLABI_DIR = $${PWD}/../..
EXTLIB_DIR = $${NEUROLABI_DIR}/lib

DEFINES += HAVE_CONFIG_H

include($${NEUROLABI_DIR}/gui/extlib.pri)

include (../../gui/gui_free.pri)
SOURCES += src/dvid_client.cpp \
    src/zdvidclient.cpp

HEADERS += \
    src/zdvidclient.h
