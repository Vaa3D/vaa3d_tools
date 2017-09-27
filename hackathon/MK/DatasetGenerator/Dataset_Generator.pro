
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = C:/Vaa3D_2010_Qt473/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= Dataset_Generator_plugin.h
SOURCES	+= Dataset_Generator_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(Dataset_Generator)
DESTDIR	= $$VAA3DPATH/bin/plugins/Dataset_Generator/
