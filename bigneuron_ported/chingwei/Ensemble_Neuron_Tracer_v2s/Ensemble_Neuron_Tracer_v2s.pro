TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= Ensemble_Neuron_Tracer_v2s.h
HEADERS += vn_imgpreprocess.h
HEADERS += fastmarching_tree.h
HEADERS += hierarchy_prune.h
HEADERS += fastmarching_dt.h
HEADERS += my_surf_objs.h

SOURCES	+= Ensemble_Neuron_Tracer_v2s.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += vn_imgpreprocess.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp
SOURCES += my_surf_objs.cpp


TARGET	= $$qtLibraryTarget(Ensemble_Neuron_Tracer_v2s)
DESTDIR	= $$VAA3DPATH/bin/plugins/bigneuron_ported/Ensemble_Neuron_Tracer_v2s/
