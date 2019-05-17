
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH  += $$VAA3DPATH/basic_c_fun
INCLUDEPATH  += $$VAA3DPATH/common_lib/include
INCLUDEPATH  += $$VAA3DPATH/common_lib/include/hdf5
INCLUDEPATH     += ../neurontracing_vn2
INCLUDEPATH     += ../neurontracing_vn2/app2
INCLUDEPATH     += ../neurontracing_vn2/app1
INCLUDEPATH     += ../neuron_image_profiling
INCLUDEPATH	+= ../eswc_converter
INCLUDEPATH	+= $$VAA3DPATH/jba/newmat11

INCLUDEPATH     += $$VAA3DPATH/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/worm_straighten_c

macx{
    LIBS += -L$$VAA3DPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat
}

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat

}



HEADERS	+= neurontracer_plugin.h
HEADERS	+= tracing_func.h

HEADERS	+= $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/readrawfile_func.h

HEADERS += ../neurontracing_vn2/vn_imgpreprocess.h
HEADERS += ../neurontracing_vn2/vn.h
HEADERS += ../neurontracing_vn2/vn_app2.h
HEADERS += ../neurontracing_vn2/vn_app1.h

HEADERS += ../neurontracing_vn2/app2/fastmarching_tree.h
HEADERS += ../neurontracing_vn2/app2/hierarchy_prune.h
HEADERS += ../neurontracing_vn2/app2/fastmarching_dt.h
HEADERS += ../neuron_image_profiling/openSWCDialog.h

SOURCES	+= neurontracer_plugin.cpp
SOURCES	+= tracing_func.cpp

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/stackutil.cpp

SOURCES	+= $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/readrawfile_func.cpp
SOURCES += $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/my_surf_objs.cpp

SOURCES += ../neurontracing_vn2/vn_imgpreprocess.cpp
SOURCES += ../neurontracing_vn2/app2_connector.cpp
SOURCES += ../neurontracing_vn2/app1_connector.cpp
SOURCES += ../neurontracing_vn2/app1/v3dneuron_gd_tracing.cpp
SOURCES += ../neurontracing_vn2/app1/gd.cpp
SOURCES += ../neurontracing_vn2/app1/calculate_cover_scores.cpp

SOURCES += ../neurontracing_vn2/swc_convert.cpp
SOURCES += ../neuron_image_profiling/profile_swc.cpp
SOURCES += ../neuron_image_profiling/compute_tubularity.cpp
SOURCES += ../neuron_image_profiling/openSWCDialog.cpp


SOURCES += $$VAA3DPATH/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$$$VAA3DPATH/neuron_editing/v_neuronswc.cpp \
    $$VAA3DPATH/graph/dijk.cpp

#meanshift
HEADERS += $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h
SOURCES += $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp

TARGET	= $$qtLibraryTarget(wp_ultratracer)
DESTDIR	= $$VAA3DPATH/../bin/plugins/wpkenanPlugin/wp_ultratracer/
