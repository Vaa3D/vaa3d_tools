
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/vs2013project/vaa3d_tools
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/released_plugins/v3d_plugins/swc_to_maskimage
HEADERS	+= review_neuron_reconstruction_plugin.h \
    review.h\
    $$VAA3DPATH/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.h \
    review_gui.h
SOURCES	+= review_neuron_reconstruction_plugin.cpp \
    reconstruction_rule.cpp\
    ../../../../v3d_external/v3d_main/basic_c_fun/basic_surf_objs.cpp \
    review.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/released_plugins/v3d_plugins/swc_to_maskimage/filter_dialog.cpp

TARGET	= $$qtLibraryTarget(review_neuron_reconstruction)
DESTDIR	= $$VAA3DPATH/bin/plugins/review_neuron_reconstruction/
