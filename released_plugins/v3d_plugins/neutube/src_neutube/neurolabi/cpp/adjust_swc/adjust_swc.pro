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

DEFINES += HAVE_CONFIG_H

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG_ _ADVANCED_
    LIBS += -lneurolabi_debug
} else {
    LIBS += -lneurolabi
}

SOURCES += src/adjust_swc.cpp $${GuiDir}/zswctree.cpp \
    $${GuiDir}/zspgrowparser.cpp $${GuiDir}/zsegmentmaparray.cpp \
    $${GuiDir}/zsuperpixelmap.cpp $${GuiDir}/zsuperpixelmaparray.cpp \
    $${GuiDir}/zvoxel.cpp $${GuiDir}/zvoxelarray.cpp $${GuiDir}/zsegmentmap.cpp \
    $${GuiDir}/zswcforest.cpp $${GuiDir}/zpoint.cpp $${GuiDir}/zswcbranch.cpp \
    $${GuiDir}/zstring.cpp $${GuiDir}/zfilelist.cpp $${GuiDir}/swctreenode.cpp\
     $${GuiDir}/zrandomgenerator.cpp $${GuiDir}/zstack.cxx $${GuiDir}/zswcpath.cpp\
     $${GuiDir}/zstackfile.cpp\
     $${GuiDir}/zsinglechannelstack.cpp $${GuiDir}/c_stack.cpp \
    $${GuiDir}/zxmldoc.cpp $${GuiDir}/zfiletype.cpp $${GuiDir}/zjsonobject.cpp \
    $${GuiDir}/zjsonvalue.cpp $${GuiDir}/zjsonparser.cpp \
    $${GuiDir}/flyem/zsynapseannotationanalyzer.cpp  \
    $${GuiDir}/flyem/zsynapseannotation.cpp \
    $${GuiDir}/flyem/zsynapseannotationarray.cpp \
    $${GuiDir}/zjsonarray.cpp $${GuiDir}/zvaa3dmarker.cpp \
    $${GuiDir}/flyem/zsynapselocationmetric.cpp \
    $${GuiDir}/flyem/zsynapseannotationmetadata.cpp \
    $${GuiDir}/flyem/zsynapselocationmatcher.cpp \
    $${GuiDir}/zinttree.cpp $${GuiDir}/zvaa3dapo.cpp

