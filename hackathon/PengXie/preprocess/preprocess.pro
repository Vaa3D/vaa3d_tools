
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/pengx/vaa3d/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/
INCLUDEPATH	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_connector/

HEADERS	+= preprocess_plugin.h
HEADERS += pre_processing_main.h
HEADERS += add_soma.h

SOURCES	+= preprocess_plugin.cpp
SOURCES += pre_processing_main.cpp
SOURCES += add_soma.cpp

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/align_axis.cpp
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/pca1.cpp
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/prune_short_branch.cpp
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/resampling.cpp
HEADERS	+= $$VAA3DPATH/released_plugins_more/v3d_plugins/blastneuron_plugin/pre_processing/sort_eswc.h

SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET	= $$qtLibraryTarget(preprocess)
DESTDIR	= $$VAA3DPATH/bin/plugins/preprocess/
