TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
QMAKE_CXXFLAGS += -std=c++11

VAA3DMAINPATH = ../../../../v3d_external/v3d_main
VAA3DPLUGINPATH = ../../../released_plugins/v3d_plugins

#
INCLUDEPATH += $$VAA3DMAINPATH/basic_c_fun
#INCLUDEPATH += $$VAA3DMAINPATH/common_lib/tiff4/include

# basic func
HEADERS += $$VAA3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.h
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

# get level 0 data
HEADERS	+= getlevel0data_plugin.h
HEADERS	+= getlevel0data_func.h

SOURCES	+= getlevel0data_plugin.cpp
SOURCES	+= getlevel0data_func.cpp

#
macx{
    QMAKE_CXXFLAGS += -stdlib=libc++ -Wno-c++11-narrowing -mmacosx-version-min=10.7
    LIBS += -lc++
    #LIBS += -L$$VAA3DMAINPATH/common_lib/tiff4/lib_mac -ltiff -lc++
}

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp -pthread
    LIBS += -fopenmp
    #LIBS += -L$$VAA3DMAINPATH/common_lib/tiff4/lib_linux -ltiff
}

TARGET	= $$qtLibraryTarget(getlevel0data)
DESTDIR	= $$VAA3DMAINPATH/../bin/plugins/image_filters/getlevel0data/

