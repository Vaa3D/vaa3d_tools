
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/common_lib/include


HEADERS	+= IVSCC_process_swc_plugin.h
HEADERS	+= ../APP2_large_scale/readRawfile_func.h

SOURCES	+= IVSCC_process_swc_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/stackutil.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_4dimage_create.cpp
SOURCES	+= ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+=  ../APP2_large_scale/readrawfile_func.cpp
SOURCES      +=	$$VAA3DPATH/basic_c_fun/mg_utilities.cpp
SOURCES      +=	$$VAA3DPATH/basic_c_fun/mg_image_lib.cpp

unix{
LIBS         += -lpthread
LIBS	     += -lv3dfftw3f -lv3dfftw3f_threads
LIBS         += -lm -L$$VAA3DPATH/common_lib/lib -lv3dtiff
}


TARGET	= $$qtLibraryTarget(IVSCC_process_swc)
DESTDIR	= $$VAA3DPATH/../bin/plugins/IVSCC/IVSCC_process_swc/
