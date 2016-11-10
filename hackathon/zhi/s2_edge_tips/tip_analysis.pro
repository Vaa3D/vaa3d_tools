
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH  += $$VAA3DPATH/common_lib/include

HEADERS	+= tip_analysis_plugin.h
SOURCES	+= tip_analysis_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/my_surf_objs.cpp
SOURCES += $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/eswc_converter/eswc_core.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(tip_analysis)
DESTDIR	= $$VAA3DPATH/../bin/plugins/s2_tip_analysis/
