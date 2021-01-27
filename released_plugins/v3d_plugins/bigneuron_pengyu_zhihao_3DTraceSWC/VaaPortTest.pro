
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d
INCLUDEPATH     += /usr/local/MATLAB/MATLAB_Runtime/v85/extern/include
#INCLUDEPATH     += /home/g/grad/zhihaozh/bigneuron/VaaTrace3DMatlab
INCLUDEPATH     += matlab_lib

#LIBS    += -L/home/g/grad/zhihaozh/bigneuron/VaaTrace3DMatlab -lvaa_port_test
LIBS    += -L matlab_lib -lvaa_port_test
LIBS    += -L/usr/local/MATLAB/MATLAB_Runtime/v85/runtime/glnxa64 -lmwmclmcrrt
#LIBS    += -L/usr/local/MATLAB/R2015a/sys/os/glnxa64 -lstdc++
#LIBS    += -L/usr/local/MATLAB/MATLAB_Runtime/v85/bin/glnxa64 -lmwmclmcrrt
LIBS    += -L/usr/local/MATLAB/MATLAB_Runtime/v85/sys/os/glnxa64 -lstdc++


HEADERS	+= VaaPortTest_plugin.h
HEADERS += func.h

SOURCES	+= VaaPortTest_plugin.cpp
SOURCES += func.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(VaaPortTest)
DESTDIR = $$V3DMAINPATH/bin/plugins/neuron_tracing/aVaaTrace3D/
