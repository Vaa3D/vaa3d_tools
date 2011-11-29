TEMPLATE = lib
CONFIG += release plugin \
    warning_off
INCLUDEPATH += ../../v3d_main/basic_c_fun /usr/include/opencv
HEADERS = steerPlugin.h \
    steerdialog.h \
    SteerableFilter3D.h \
    polynomial.h \
    Cube.h \
    utils.h
SOURCES = steerPlugin.cpp \
    steerdialog.cpp \
    SteerableFilter3D.cpp \
    utils.cpp
LIBS += `pkg-config --libs opencv`
TARGET = $$qtLibraryTarget(steerableFilters3d)
DESTDIR = ../../v3d/plugins/steerableFilters3d
FORMS += steerdialog.ui
QMAKE_CXXFLAGS += -fopenmp
QMAKE_LFLAGS += -fopenmp
