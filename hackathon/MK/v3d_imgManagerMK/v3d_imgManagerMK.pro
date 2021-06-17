QT -= gui

TARGET = v3d_imgManagerMK
TEMPLATE = lib
CONFIG += qt plugin warn_off
CONFIG += staticlib
QMAKE_CXXFLAGS += /MP

DEFINES += V3D_IMGMANAGERMK_LIBRARY
DEFINES += _CRT_SECURE_NO_WARNINGS
DEFINES += TEEM_STATIC

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR = ./

VAA3DPATH = ../../../../v3d_external
V3DTOOLPATH = ../../../released_plugins/v3d_plugins
INCLUDEPATH += $$(BOOST_PATH)
INCLUDEPATH += $$(QTDIR)/include
INCLUDEPATH += $$(QTDIR)/include/QtCore
INCLUDEPATH += $$(QTDIR)/include/QtGui
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/v3d_main/v3d
INCLUDEPATH += $$VAA3DPATH/v3d_main/neuron_editing
INCLUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH += $$V3DTOOLPATH/swc2mask_cylinder
INCLUDEPATH += $$VAA3DPATH/v3d_main/io
INCLUDEPATH += ../v3dSource
INCLUDEPATH += ../v3d_imgManagerMK
INCLUDEPATH += ../v3d_imgManagerMK/imgProcessor
INCLUDEPATH += ../v3d_imgManagerMK/imgAnalyzer

LIBS += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibtiff 
LIBS += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -lteem

SOURCES += ./ImgManager.cpp
SOURCES += ./processManager.cpp
SOURCES += ./integratedDataStructures.cpp
SOURCES += ./imgAnalyzer/ImgAnalyzer.cpp
SOURCES += ./imgProcessor/ImgProcessor.cpp
SOURCES += $$V3DTOOLPATH/swc2mask_cylinder/my_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES += $$VAA3DPATH/v3d_main/io/v3d_nrrd.cpp
SOURCES += ../v3dSource/mg_utilities.cpp
SOURCES += ../v3dSource/stackutil.cpp
SOURCES += ../v3dSource/basic_4dimage.cpp

HEADERS += ./ImgManager.h
HEADERS += ./processManager.h
HEADERS += ./integratedDataStructures.h
HEADERS += ./v3d_imgManagerMK_Define.h
HEADERS += ./imgAnalyzer/ImgAnalyzer.h
HEADERS += ./imgProcessor/ImgProcessor.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage.h
HEADERS += $$V3DTOOLPATH/swc2mask_cylinder/my_surf_objs.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.h