TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH =  C:/Users/Braintell/Desktop/v3d_external-Qt6_Win10/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
#INCLUDEPATH	+= $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/eswc_converter
INCLUDEPATH	+= $$VAA3DPATH/jba/newmat11
QT += widgets
macx{
    LIBS += -L$$VAA3DPATH/common_lib/lib_mac64 -lv3dnewmat
#    CONFIG += x86_64
}

win32{
    LIBS += -L$$VAA3DPATH/common_lib/mingw -lv3dnewmat
}

unix:!macx {
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat
}


HEADERS	+= neuron_image_profiling_plugin.h \
    compute_tubularity.h
SOURCES	+= neuron_image_profiling_plugin.cpp \
    compute_tubularity.cpp
SOURCES	+= profile_swc.cpp
HEADERS	+= profile_swc.h
SOURCES += openSWCDialog.cpp
HEADERS += openSWCDialog.h
SOURCES += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/eswc_converter/eswc_core.cpp
HEADERS += $$VAA3DPATH/../vaa3d_tools/released_plugins/v3d_plugins/eswc_converter/eswc_core.h


SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
HEADERS	+= $$VAA3DPATH/basic_c_fun/v3d_message.h





TARGET	= $$qtLibraryTarget(neuron_image_profiling)
DESTDIR	= $$VAA3DPATH/bin/plugins/neuron_utilities/neuron_image_profiling/
