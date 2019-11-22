
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/vs2013project/vaa3d_tools
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/v3d
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH     += $$VAA3DPATH/v3d_main/jba/newmat11
INCLUDEPATH     += $$VAA3DPATH/released_plugins/v3d_plugins/swc_to_maskimage
LIBS            += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat
HEADERS	+= soma_branch_plugin.h \
    $$VAA3DPATH/v3d_main/v3d/compute_win_pca.h\
    branch_count_soma.h \
    branch_count_gui.h\
    $$VAA3DPATH/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h
HEADERS+=$$VAA3DPATH/v3d_main/basic_c_fun/basic_landmark.h\
    $$VAA3DPATH/v3d_main/jba/newmat11/newmatap.h\
    $$VAA3DPATH/v3d_main/jba/newmat11/newmatio.h

SOURCES	+= soma_branch_plugin.cpp \
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp\
    branch_count_soma.cpp \
    branch_count_menu.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.cpp
TARGET	= $$qtLibraryTarget(soma_branch)
DESTDIR	= $$VAA3DPATH/bin/plugins/soma_branch/
