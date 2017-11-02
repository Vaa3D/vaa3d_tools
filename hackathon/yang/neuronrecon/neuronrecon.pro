TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
QMAKE_CXXFLAGS += -std=c++11

VAA3DMAINPATH = ../../../../v3d_external/v3d_main
VAA3DPLUGINPATH = ../../../released_plugins/v3d_plugins

# set with your evironment settings
HOMEDIR = /home/yangy/work/local
INCLUDEPATH     += $${HOMEDIR}/include
INCLUDEPATH     += $${HOMEDIR}/include/ITK-4.13
INCLUDEPATH     += $${HOMEDIR}/include/eigen3

#
INCLUDEPATH	+= $$VAA3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$VAA3DMAINPATH/neuron_editing
INCLUDEPATH     += $$VAA3DMAINPATH/common_lib/include/boost/graph
INCLUDEPATH     += $$VAA3DMAINPATH/common_lib/include

INCLUDEPATH     += ./3rdparty

INCLUDEPATH     += $$VAA3DPLUGINPATH/neurontracing_vn2
INCLUDEPATH     += $$VAA3DPLUGINPATH/neurontracing_vn2/app2

# basic func
HEADERS += $$VAA3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/basic_4dimage.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS += $$VAA3DMAINPATH/neuron_editing/neuron_sim_scores.h
HEADERS += $$VAA3DMAINPATH/common_lib/include/boost/graph/prim_minimum_spanning_tree.hpp
HEADERS += $$VAA3DMAINPATH/common_lib/include/boost/graph/kruskal_min_spanning_tree.hpp
HEADERS += $$VAA3DMAINPATH/common_lib/include/boost/graph/adjacency_list.hpp
HEADERS += $$VAA3DMAINPATH/common_lib/include/boost/config.hpp
HEADERS	+= $$VAA3DMAINPATH/neuron_editing/v_neuronswc.h

SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DMAINPATH/neuron_editing/v_neuronswc.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/basic_4dimage.cpp

# neuron tree(s) construction
HEADERS	+= img.h
HEADERS	+= neuronrecon_plugin.h
HEADERS	+= neuronrecon_func.h
HEADERS	+= neuronrecon.h

SOURCES	+= img.cxx
SOURCES	+= neuronrecon_plugin.cpp
SOURCES	+= neuronrecon_func.cpp
SOURCES	+= neuronrecon.cpp

# app2
HEADERS += $$VAA3DPLUGINPATH/neurontracing_vn2/vn_imgpreprocess.h
HEADERS += $$VAA3DPLUGINPATH/neurontracing_vn2/vn.h
HEADERS += $$VAA3DPLUGINPATH/neurontracing_vn2/vn_app2.h
HEADERS	+= $$VAA3DPLUGINPATH/neurontracing_vn2/app2/my_surf_objs.h
HEADERS += $$VAA3DPLUGINPATH/neurontracing_vn2/app2/fastmarching_tree.h
HEADERS += $$VAA3DPLUGINPATH/neurontracing_vn2/app2/hierarchy_prune.h
HEADERS += $$VAA3DPLUGINPATH/neurontracing_vn2/app2/fastmarching_dt.h

SOURCES += $$VAA3DPLUGINPATH/neurontracing_vn2/vn_imgpreprocess.cpp
SOURCES += $$VAA3DPLUGINPATH/neurontracing_vn2/app2_connector.cpp

macx{
    QMAKE_CXXFLAGS += -stdlib=libc++ -Wno-c++11-narrowing
    LIBS += -L$$VAA3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$VAA3DMAINPATH/jba/c++ -lv3dnewmat
    LIBS += -framework OpenCL
    LIBS += -lc++
    LIBS += -litksys-4.13 -lITKVNLInstantiation-4.13 -litkvnl_algo-4.13 -litkvnl-4.13 -litkv3p_netlib-4.13 -litknetlib-4.13 -litkvcl-4.13
    LIBS += -lITKCommon-4.13 -lITKIOImageBase-4.13 -lITKIOTIFF-4.13 -lITKGPUAnisotropicSmoothing-4.13 -lITKGPUFiniteDifference-4.13 -lITKGPUCommon-4.13
    LIBS += -L$$VAA3DMAINPATH/common_lib/src_packages/mylib_tiff -lmylib
}

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp -pthread
    LIBS += -fopenmp
    LIBS += -L$$VAA3DMAINPATH/jba/c++ -lv3dnewmat
    LIBS += -lOpenCL
    LIBS += -lITKIOTIFF-4.13 -lITKGPUCommon-4.13 -lITKGPUAnisotropicSmoothing-4.13
}

# deep learning detection
#HEADERS += zhidl/classification.h
#HEADERS += ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.h
#SOURCES += zhidl/classification.cpp
#SOURCES += ../../../released_plugins/v3d_plugins/mean_shift_center/mean_shift_fun.cpp
#INCLUDEPATH += /home/yangy/work/cuda/include
#LIBS += -lopencv_core -lopencv_imgproc -lopencv_highgui
#LIBS += -lcaffe -lhdf5 -lhdf5_cpp
#LIBS += -lglog -lgflags -lprotobuf -llmdb -lleveldb -lstdc++ -lcudnn -lcblas -latlas
#LIBS += -L/home/yangy/work/local/lib -lboost_system -lboost_filesystem -lboost_thread
#LIBS += -L/home/yangy/work/cuda/lib64 -lcudart -lcublas -lcurand

TARGET	= $$qtLibraryTarget(neurontree_construct)
DESTDIR	= $$VAA3DMAINPATH/../bin/plugins/neuron_utilities/neurontree_construct/
