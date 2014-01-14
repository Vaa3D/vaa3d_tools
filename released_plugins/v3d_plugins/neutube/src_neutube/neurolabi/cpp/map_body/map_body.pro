TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CDir = $${PWD}/../../c
GuiDir = $${PWD}/../../gui
GenelibDir = $${PWD}/../../lib/genelib/src

HOME = $$system(echo $HOME)
INCLUDEPATH += $${CDir} $${CDir}/include $${GuiDir} $${GenelibDir} \
    /usr/include/libxml2 $${HOME}/local/include

LIBS += -L/usr/local/lib \
    -L/usr/lib \
    -L$${CDir}/lib \
    -L$${HOME}/local/lib \
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

include (../../gui/gui_free.pri)

SOURCES += src/map_body.cpp

#$${GuiDir}/zswctree.cpp\
#  $${GuiDir}/zspgrowparser.cpp $${GuiDir}/zsegmentmaparray.cpp\
#  $${GuiDir}/zsuperpixelmap.cpp $${GuiDir}/zsuperpixelmaparray.cpp\
#  $${GuiDir}/zvoxel.cpp $${GuiDir}/zvoxelarray.cpp $${GuiDir}/zsegmentmap.cpp\
#  $${GuiDir}/zswcforest.cpp $${GuiDir}/zpoint.cpp $${GuiDir}/zswcbranch.cpp\
#  $${GuiDir}/zstring.cpp $${GuiDir}/zfilelist.cpp $${GuiDir}/swctreenode.cpp \
#  $${GuiDir}/zrandomgenerator.cpp $${GuiDir}/zstack.cxx $${GuiDir}/zswcpath.cpp\
#  $${GuiDir}/zstackfile.cpp \
#  $${GuiDir}/zsinglechannelstack.cpp $${GuiDir}/c_stack.cpp\
#  $${GuiDir}/zxmldoc.cpp $${GuiDir}/zfiletype.cpp $${GuiDir}/zjsonobject.cpp\
#  $${GuiDir}/zjsonvalue.cpp $${GuiDir}/zjsonparser.cpp\
#  $${GuiDir}/zhdf5reader.cpp $${GuiDir}/mylib/array.cpp\
#  $${GuiDir}/mylib/mylib.c $${GuiDir}/mylib/utilities.cpp $${GuiDir}/zgraph.cpp\
#  $${GuiDir}/zcuboid.cpp $${GuiDir}/zweightedpoint.cpp \
#  $${GuiDir}/zweightedpointarray.cpp $${GuiDir}/zobject3dscan.cpp\
#  $${GuiDir}/zobject3d.cpp $${GuiDir}/zobject3darray.cpp\
#  $${GuiDir}/zresolution.cpp $${GuiDir}/zprogressable.cpp \
#  $${GuiDir}/zprogressReporter.cpp $${GuiDir}/zinttree.cpp \
#  $${GuiDir}/neutubeconfig.cpp $${GuiDir}/zhistogram.cpp \
#  $${GuiDir}/zlogmessagereporter.cpp $${GuiDir}/zmessagereporter.cpp \
#  $${GuiDir}/c_json.cpp
