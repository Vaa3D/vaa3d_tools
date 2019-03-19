
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

HEADERS	+= reco_eval_plugin.h
SOURCES	+= reco_eval_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

#mask generation
HEADERS	+= $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h
SOURCES	+= $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.cpp

TARGET	= $$qtLibraryTarget(reco_eval)
DESTDIR	= $$VAA3DPATH/../bin/plugins/reco_eval/
