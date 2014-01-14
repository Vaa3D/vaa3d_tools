TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CDir = $${PWD}/../../c
GuiDir = $${PWD}/../../gui
GenelibDir = $${PWD}/../../lib/genelib/src

INCLUDEPATH += $${CDir} $${CDir}/include $${GuiDir} $${GenelibDir} \
    /usr/include/libxml2

LIBS += -L/usr/local/lib \
    -L/usr/lib \
    -L$${CDir}/lib \
    -lfftw3 \
    -lfftw3f \
    -lxml2 \
    -lpng

DEFINES += HAVE_CONFIG_H

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG_ _ADVANCED_
    LIBS += -lneurolabi_debug
} else {
    LIBS += -lneurolabi
}

SOURCES += src/sort_body_id.cpp $${GuiDir}/zstring.cpp \
   $${GuiDir}/zargumentprocessor.cpp
