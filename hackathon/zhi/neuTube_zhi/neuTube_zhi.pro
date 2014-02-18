
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += /home/zhi/work/vaa3d_tools/released_plugins/v3d_plugins/neutube/src_neutube/neurolabi/c
INCLUDEPATH     += /home/zhi/work/vaa3d_tools/released_plugins/v3d_plugins/neutube/src_neutube/neurolabi/gui
INCLUDEPATH     += /home/zhi/work/vaa3d_tools/released_plugins/v3d_plugins/neutube/src_neutube/neurolabi/c/include
INCLUDEPATH     +=/home/zhi/work/vaa3d_tools/released_plugins/v3d_plugins/neutube/src_neutube/neurolabi/lib/genelib/src

HEADERS	+= neuTube_zhi_plugin.h

SOURCES	+= neuTube_zhi_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp

SOURCES += /home/zhi/Dropbox/neuTube/neurolabi/lib/genelib/src/image_lib.c
SOURCES += /home/zhi/work/vaa3d_tools/released_plugins/v3d_plugins/neutube/src_neutube/neurolabi/gui/mylib/utilities.cpp

TARGET	= $$qtLibraryTarget(neuTube_zhi)
DESTDIR	= ../../../../v3d_external/bin/plugins/neuTube_zhi/
