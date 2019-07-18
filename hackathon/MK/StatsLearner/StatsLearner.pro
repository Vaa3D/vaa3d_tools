#-------------------------------------------------
#
# Project created by QtCreator 2018-12-19T12:33:58
#
#-------------------------------------------------

QT       -= gui

TARGET = StatsLearner
TEMPLATE = lib
CONFIG	+= qt plugin warn_off
QMAKE_CXXFLAGS += -std=c++11

DEFINES += STATSLEARNER_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = ../../../../v3d_external/v3d_main/common_lib/lib

VAA3DPATH = ../../../../v3d_external
V3DTOOLPATH = ../../../released_plugins/v3d_plugins
INCLUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$V3DTOOLPATH/swc2mask_cylinder
INCLUDEPATH += /usr/local/Trolltech/Qt-4.7.3/include/QtGui
INCLUDEPATH += /usr/local/Trolltech/Qt-4.7.3/include/QtCore
INCLUDEPATH += ../v3d_imgManagerMK
INCLUDEPATH += ../v3d_imgManagerMK/imgProcessor
INCLUDEPATH += ../v3d_imgManagerMK/imgAnalyzer

HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS += $$V3DTOOLPATH/swc2mask_cylinder/my_surf_objs.h
HEADERS += ./StatsLearningManager.h
HEADERS += ./FeatureExtractor.h
HEADERS += ../v3d_imgManagerMK/ImgManager.h
HEADERS += ../v3d_imgManagerMK/imgAnalyzer/ImgAnalyzer.h

SOURCES += ./StatsLearningManager.cpp
SOURCES += ./FeatureExtractor.cpp
