
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external

INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/v3d_main/v3d
INCLUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH += $$(BOOST_PATH)
INCLUDEPATH += ../NeuronStructNavigator
INCLUDEPATH += ../v3d_imgManagerMK

HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h
HEADERS	+= swcFileRename_plugin.h
HEADERS += swcRenameDlg.h
HEADERS += FileNameChangerIndexer.h
HEADERS += ReconOperator.h

SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= swcFileRename_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += swcRenameDlg.cpp
SOURCES += FileNameChangerIndexer.cpp
SOURCES += ReconOperator.cpp

LIBS += -L../NeuronStructNavigator -lNeuronStructNavigator

FORMS += renameSWC.ui

TARGET	= $$qtLibraryTarget(NeuronReconFile_Manager)
DESTDIR	= $$VAA3DPATH/bin/plugins/NeuronReconFile_Manager/
