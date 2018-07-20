TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
QMAKE_CXXFLAGS += -std=c++11

VAA3DMAINPATH = ../../../../v3d_external/v3d_main
VAA3DPLUGINPATH = ../../../released_plugins/v3d_plugins

#
INCLUDEPATH += $$VAA3DMAINPATH/basic_c_fun
INCLUDEPATH += $$VAA3DMAINPATH/common_lib/tiff4/include

# basic func
HEADERS += $$VAA3DMAINPATH/basic_c_fun/v3d_message.h
#HEADERS += $$VAA3DMAINPATH/basic_c_fun/basic_4dimage.h
#HEADERS += $$VAA3DMAINPATH/basic_c_fun/stackutil.h

SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/v3d_message.cpp
#SOURCES += $$VAA3DMAINPATH/basic_c_fun/basic_4dimage.cpp
#SOURCES += $$VAA3DMAINPATH/basic_c_fun/stackutil.cpp

# sample 2 images
HEADERS	+= sample2images_plugin.h
HEADERS	+= sample2images_func.h

SOURCES	+= sample2images_plugin.cpp
SOURCES	+= sample2images_func.cpp

#
macx{
    QMAKE_CXXFLAGS += -stdlib=libc++ -Wno-c++11-narrowing -mmacosx-version-min=10.7
    LIBS += -L$$VAA3DMAINPATH/common_lib/tiff4/lib_mac -ltiff -lc++
}

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp -pthread
    LIBS += -fopenmp
    LIBS += -L$$VAA3DMAINPATH/common_lib/tiff4/lib_linux -ltiff
}

TARGET	= $$qtLibraryTarget(sample2images)
DESTDIR	= $$VAA3DMAINPATH/../bin/plugins/image_filters/sample2images/
