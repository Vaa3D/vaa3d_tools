
#TEMPLATE        = lib
TEMPLATE        = app
#CONFIG  += qt plugin warn_off
CONFIG  += qt application warn_off
#CONFIG += x86_64
VAA3DPATH = ~/v3d_external
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/v3d_main/jba/

#HEADERS += consensusbuilder_plugin.h
#SOURCES += consensusbuilder_plugin.cpp

HEADERS = Reconstruction.h
HEADERS += Composite.h
HEADERS += ActionHook.h
HEADERS += ConsensusBuilder.h
HEADERS += logger.h
HEADERS += align.h

SOURCES = Reconstruction.cpp
SOURCES += Composite.cpp
SOURCES += ActionHook.cpp
SOURCES += ConsensusBuilder.cpp
SOURCES += main.cpp
SOURCES += logger.cpp
SOURCES += align.cpp

SOURCES += tree_matching/my_surf_objs.cpp
SOURCES += tree_matching/neuron_tree_align.cpp
SOURCES += tree_matching/seg_weight.cpp
SOURCES += tree_matching/swc_utils.cpp


win32 {
   contains(QMAKE_HOST.arch, x86_64) {
      LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat
   } else {
      LIBS     += -L$$VAA3DPATH/v3d_main/common_lib/winlib -llibnewmat
   }
}

unix {
#    LIBS += -L$$VAA3DPATH/v3d_main/jba/c++ -lv3dnewmat
#    LIBS += -L$$VAA3DPATH/v3d_main/jba/newmat11 -lv3dnewmat
}
#TARGET  = $$qtLibraryTarget(consensusbuilder)
TARGET  = consensusbuilder
#DESTDIR = $$VAA3DPATH/bin/plugins/consensusbuilder/
DESTDIR = /Users/toddgillette/workspace/ConsensusBuilder/bin/
