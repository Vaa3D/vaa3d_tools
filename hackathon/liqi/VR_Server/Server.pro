QT += core
QT += network
QT += gui

CONFIG += c++11
VAA3DPATH = ../../../v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

TARGET = Server
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    server.cpp

HEADERS += \
    server.h
