
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun

HEADERS	+= Node_type_correction_plugin.h
HEADERS += Node_type_correction_func.h
HEADERS	+= $$VAA3DPATH/basic_c_fun/v3d_message.h
HEADERS += $$VAA3DPATH/basic_c_fun/basic_surf_objs.h
HEADERS += $$VAA3DPATH/basic_c_fun/v3d_interface.h

SOURCES	+= Node_type_correction_plugin.cpp
SOURCES += Node_type_correction_func.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp


TARGET	= $$qtLibraryTarget(Node_type_correction)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_utilities/Node_type_correction/
