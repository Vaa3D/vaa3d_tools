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
    -lpng \
    -ljansson

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG_ _ADVANCED_
    LIBS += -lneurolabi_debug
} else {
    LIBS += -lneurolabi
}

SOURCES += src/psd_compare.cpp \
    $${GuiDir}/zpoint.cpp \
    $${GuiDir}/zobject3darray.cpp \
    $${GuiDir}/zobject3d.cpp \
    $${GuiDir}/zstring.cpp $${GuiDir}/zargumentprocessor.cpp \
    $${GuiDir}/zkmeansclustering.cpp $${GuiDir}/zdoublevector.cpp \
    $${GuiDir}/zrandomgenerator.cpp $${GuiDir}/swctreenode.cpp \
    $${GuiDir}/zswctree.cpp $${GuiDir}/zswcbranch.cpp \
    $${GuiDir}/flyem/zsynapselocationmatcher.cpp \
    $${GuiDir}/zswcforest.cpp $${GuiDir}/flyem/zsynapseannotation.cpp \
    $${GuiDir}/flyem/zsynapseannotationarray.cpp \
    $${GuiDir}/zjsonarray.cpp $${GuiDir}/zjsonvalue.cpp $${GuiDir}/zjsonobject.cpp \
    $${GuiDir}/zvaa3dmarker.cpp $${GuiDir}/flyem/zsynapseannotationmetadata.cpp \
    $${GuiDir}/zfilelist.cpp $${GuiDir}/zvaa3dapo.cpp \
    $${GuiDir}/flyem/zsynapseannotationanalyzer.cpp \
    $${GuiDir}/flyem/zsynapselocationmetric.cpp $${GuiDir}/zinttree.cpp \
    $${GuiDir}/zswcpath.cpp $${GuiDir}/zjsonparser.cpp $${GuiDir}/zfiletype.cpp
