TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

CDir = $${PWD}/../../c
GuiDir = $${PWD}/../../gui
GenelibDir = $${PWD}/../../lib/genelib/src

INCLUDEPATH += $${CDir} $${CDir}/include $${GuiDir} $${GenelibDir} \
    /usr/include/libxml2 /usr/local/include/libxml2


DEFINES += HAVE_CONFIG_H

CONFIG(debug, debug|release) {
    DEFINES += _DEBUG_ _ADVANCED_
    LIBS += -lneurolabi_debug
} else {
    LIBS += -lneurolabi
}

LIBS += -L/usr/local/lib \
    -L/usr/lib \
    -L$${CDir}/lib \
    -lfftw3 \
    -lfftw3f \
    -lxml2 \
    -lpng \
    -ljansson

#for specific machines
HOSTNAME = $$system(echo $HOSTNAME)

contains(HOSTNAME, zhaot-ws1.janelia.priv) {
  LOCAL_INSTALL_DIR = /groups/flyem/home/zhaot/local
  INCLUDEPATH += $${LOCAL_INSTALL_DIR}/include
  LIBS += -L$${LOCAL_INSTALL_DIR}/lib
  exists($${LOCAL_INSTALL_DIR}/include/jansson.h) {
    exists($${LOCAL_INSTALL_DIR}/lib/libjansson.a) | exists($${LOCAL_INSTALL_DIR}/lib/libjansson.so) {
      DEFINES *= _ENABLE_JANSSON_
      LIBS *= -ljansson
    }
  }
}

SOURCES += src/skeletonize.cpp $${GuiDir}/zswctree.cpp \
      $${GuiDir}/zspgrowparser.cpp $${GuiDir}/zsegmentmaparray.cpp \
      $${GuiDir}/zsuperpixelmap.cpp $${GuiDir}/zsuperpixelmaparray.cpp \
      $${GuiDir}/zvoxel.cpp $${GuiDir}/zvoxelarray.cpp $${GuiDir}/zsegmentmap.cpp \
      $${GuiDir}/zswcforest.cpp $${GuiDir}/zpoint.cpp $${GuiDir}/zswcbranch.cpp \
      $${GuiDir}/zstring.cpp $${GuiDir}/zfilelist.cpp $${GuiDir}/swctreenode.cpp\
      $${GuiDir}/zrandomgenerator.cpp $${GuiDir}/zstack.cxx $${GuiDir}/zswcpath.cpp\
      $${GuiDir}/zstackfile.cpp\
      $${GuiDir}/zsinglechannelstack.cpp $${GuiDir}/c_stack.cpp \
      $${GuiDir}/zxmldoc.cpp $${GuiDir}/zfiletype.cpp $${GuiDir}/zjsonobject.cpp \
      $${GuiDir}/zjsonvalue.cpp $${GuiDir}/zjsonparser.cpp\
      $${GuiDir}/zhdf5reader.cpp $${GuiDir}/mylib/array.cpp\
      $${GuiDir}/mylib/mylib.c $${GuiDir}/mylib/utilities.cpp $${GuiDir}/zgraph.cpp\
      $${GuiDir}/zcuboid.cpp $${GuiDir}/zweightedpoint.cpp \
      $${GuiDir}/zweightedpointarray.cpp $${GuiDir}/zstackskeletonizer.cpp\
      $${GuiDir}/zobject3dscan.cpp $${GuiDir}/zobject3d.cpp $${GuiDir}/zobject3darray.cpp\
      $${GuiDir}/zresolution.cpp $${GuiDir}/zprogressable.cpp $${GuiDir}/zprogressreporter.cpp\
      $${GuiDir}/zinttree.cpp $${GuiDir}/neutubeconfig.cpp $${GuiDir}/zhistogram.cpp\
      $${GuiDir}/zlogmessagereporter.cpp $${GuiDir}/zmessagereporter.cpp\
      $${GuiDir}/c_json.cpp $${GuiDir}/zswcresampler.cpp $${GuiDir}/zswcgenerator.cpp\
      $${GuiDir}/flyem/zflyemneuronrange.cpp $${GuiDir}/flyem/zflyemneuronaxis.cpp\
      $${GuiDir}/flyem/zflyemneuronrangecompare.cpp
