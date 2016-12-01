
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = /home/yang/vaa3d/vaa3d_tools/hackathon/yangjian/blastneuron_bjut_yang
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun

HEADERS	+= blastneuron_bjut_yang_plugin.h
SOURCES	+= blastneuron_bjut_yang_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(blastneuron_bjut_yang)
DESTDIR	= $$VAA3DPATH/bin/plugins/blastneuron_bjut_yang/
