
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = D:/vaa3d_build/v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/neurontracing_vn2
INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/neurontracing_vn2/app2
INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/mean_shift_center
INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_reliability_score/src
#INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_image_profiling
INCLUDEPATH     += $$VAA3DPATH/v3d_main/common_lib/include
INCLUDEPATH     += $$VAA3DPATH/v3d_main/jba/newmat11
INCLUDEPATH     += $$VAA3DPATH/v3d_main/jba/c+
INCLUDEPATH     += $$VAA3DPATH/released_plugins_more/v3d_plugins/sort_neuron_swc




HEADERS	+= tera_retrace_plugin.h \
    tera_retrace_func.h \
HEADERS += $$VAA3DPATH/released_plugins_more/v3d_plugins/sort_neuron_swc/sort_swc.h
SOURCES	+= tera_retrace_plugin.cpp \
    tera_retrace_func.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/stackutil.cpp

SOURCES	+= $$VAA3DPATH/../vaa3d_tools/hackathon/zhi/APP2_large_scale/readrawfile_func.cpp
SOURCES += $$VAA3DPATH/../vaa3d_tools/hackathon/zhi/APP2_large_scale/my_surf_objs.cpp

SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.cpp
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/neurontracing_vn2/app2_connector.cpp
SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/neurontracing_vn2/swc_convert.cpp
#SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_image_profiling/profile_swc.cpp
#SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_image_profiling/compute_tubularity.cpp
#SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/neuron_image_profiling/openSWCDialog.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$VAA3DPATH/v3d_main/neuron_editing/v_neuronswc.cpp
SOURCES += $$VAA3DPATH/v3d_main/graph/dijk.cpp


SOURCES += $$VAA3DPATH/released_plugins_more/v3d_plugins/mean_shift_center/mean_shift_fun.cpp




TARGET	= $$qtLibraryTarget(tera_retrace)
DESTDIR	= $$VAA3DPATH/bin/plugins/tera_retrace/

#win32 {
#LIBS += -L$$V3DMAINPATH/v3d_main/common_lib/winlib -llibtiff
#LIBS += -L$$V3DMAINPATH/v3d_main/common_lib/winlib -llibnewmat
##LIBS += -L$$V3DMAINPATH/common_lib/src_packages/mylib_tiff -llibmylib
#LIBS += -L$$V3DMAINPATH/v3d_main/jba/c++
##LIBS += -lm
#}



win32 {
LIBS += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibtiff
LIBS += -L$$VAA3DPATH/v3d_main/common_lib/winlib64 -llibnewmat
}
