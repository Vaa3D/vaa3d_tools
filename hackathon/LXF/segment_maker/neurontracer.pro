
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH  += $$VAA3DPATH/basic_c_fun
INCLUDEPATH  += $$VAA3DPATH/common_lib/include
INCLUDEPATH  += $$VAA3DPATH/common_lib/include/hdf5
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1
INCLUDEPATH     += ../../../released_plugins/v3d_plugins/neuron_image_profiling
INCLUDEPATH	+= ../../../released_plugins/v3d_plugins/eswc_converter
INCLUDEPATH	+= $$VAA3DPATH/jba/newmat11

INCLUDEPATH     += $$VAA3DPATH/neuron_editing
INCLUDEPATH     += $$VAA3DPATH/worm_straighten_c
win32 {
    LIBS += -L$$VAA3DPATH/common_lib/winlib64 -llibtiff
}
macx{
    LIBS += -L$$VAA3DPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat
}

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
    LIBS += -L$$VAA3DPATH/jba/c++ -lv3dnewmat

}



HEADERS	+= neurontracer_plugin.h \
    ../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h \
    ../anisodiffusion_LXF/src/q_imgaussian3D.h \
    ../anisodiffusion_LXF/src/q_EigenVectors3D.h \
    ../anisodiffusion_LXF/src/q_derivatives3D.h \
    ../anisodiffusion_LXF/src/q_AnisoDiff3D.h \
    ../anisodiffusion_LXF/src/EigenDecomposition3.h \
    ../../../../v3d_external/v3d_main/basic_c_fun/stackutil.h
HEADERS	+= tracing_func.h

HEADERS	+= $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/readrawfile_func.h

HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app2.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_app1.h

HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/hierarchy_prune.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_dt.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/fastmarching_tree.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/marker_radius.h
HEADERS += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2/my_surf_objs.h
HEADERS += ../../../released_plugins/v3d_plugins/neuron_image_profiling/openSWCDialog.h

SOURCES	+= neurontracer_plugin.cpp \
    ../anisodiffusion_LXF/src/q_imgaussian3D.cpp \
    ../anisodiffusion_LXF/src/q_EigenVectors3D.cpp \
    ../anisodiffusion_LXF/src/q_derivatives3D.cpp \
    ../anisodiffusion_LXF/src/q_AnisoDiff3D.cpp \
    ../anisodiffusion_LXF/src/EigenDecomposition3.cpp
SOURCES	+= tracing_func.cpp

SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/stackutil.cpp

SOURCES	+= $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/readrawfile_func.cpp
SOURCES += $$VAA3DPATH/../../vaa3d_tools/hackathon/zhi/APP2_large_scale/my_surf_objs.cpp

SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/vn_imgpreprocess.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app2_connector.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/app1/calculate_cover_scores.cpp

SOURCES += ../../../released_plugins/v3d_plugins/neurontracing_vn2/swc_convert.cpp
SOURCES += ../../../released_plugins/v3d_plugins/neuron_image_profiling/openSWCDialog.cpp


SOURCES += $$VAA3DPATH/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$$$VAA3DPATH/neuron_editing/v_neuronswc.cpp \
    $$VAA3DPATH/graph/dijk.cpp

#meanshift
HEADERS += $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h
SOURCES += $$VAA3DPATH/../../vaa3d_tools/released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp

TARGET	= $$qtLibraryTarget(segment_maker)
DESTDIR	= $$VAA3DPATH/../bin/plugins/neuron_tracing/segment_maker/
