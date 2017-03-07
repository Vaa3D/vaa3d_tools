
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH  += $$VAA3DPATH/v3d_main/common_lib/include


HEADERS	+= mapping3D_swc_plugin.h
HEADERS += openSWCDialog.h


SOURCES	+= mapping3D_swc_plugin.cpp
SOURCES += openSWCDialog.cpp
SOURCES += ../../../released_plugins/v3d_plugins/bigneuron_zz_neurontracing_TReMAP/my_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/stackutil.cpp
SOURCES      +=	$$VAA3DPATH/v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES      +=	$$VAA3DPATH/v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES += ../APP2_large_scale/readrawfile_func.cpp


SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

win32{
    LIBS         += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibtiff
    LIBS	 += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibfftw3f-3
}

unix{
    LIBS         += -lm -L$$VAA3DPATH/v3d_main/common_lib/lib -lv3dtiff
   # LIBS         += -lpthread
   # LIBS	 += -lv3dfftw3f -lv3dfftw3f_threads
}



TARGET	= $$qtLibraryTarget(mapping3D_swc)
DESTDIR	= $$VAA3DPATH/bin/plugins/mapping3D_swc/
