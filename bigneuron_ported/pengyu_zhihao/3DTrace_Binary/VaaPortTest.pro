
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/g/grad/zhihaozh/bigneuron/vaa3d_source/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += /usr/local/MATLAB/R2015a/extern/include
INCLUDEPATH     += /home/g/grad/zhihaozh/bigneuron/matlab

LIBS    += -L/home/g/grad/zhihaozh/bigneuron/matlab -lvaa_port_test
LIBS    += -L/usr/local/MATLAB/R2015a/runtime/glnxa64 -lmwmclmcrrt
LIBS    += -L/usr/local/MATLAB/R2015a/sys/os/glnxa64 -lstdc++


HEADERS	+= VaaPortTest_plugin.h
HEADERS += func.h

SOURCES	+= VaaPortTest_plugin.cpp
SOURCES += func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(VaaPortTest)
DESTDIR = /home/g/grad/zhihaozh/bigneuron/vaa3d_redhat_fedora_ubuntu_64bit_v2.921/plugins/aVaaPortTest/
