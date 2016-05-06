
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += /usr/local/MATLAB/R2015a/extern/include
#INCLUDEPATH     += /home/g/grad/zhihaozh/bigneuron/VaaTrace3DMatlab
INCLUDEPATH     += matlab_lib

#LIBS    += -L/home/g/grad/zhihaozh/bigneuron/VaaTrace3DMatlab -lvaa_port_test
LIBS    += -L matlab_lib -lvaa_port_test
LIBS    += -L/usr/local/MATLAB/R2015a/runtime/glnxa64 -lmwmclmcrrt
LIBS    += -L/usr/local/MATLAB/R2015a/sys/os/glnxa64 -lstdc++


HEADERS	+= VaaPortTest_plugin.h
HEADERS += func.h

SOURCES	+= VaaPortTest_plugin.cpp
SOURCES += func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(VaaPortTest)
DESTDIR = A3DPATH/bin/plugins/aVaaTrace3D/
