TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

QMAKE_CXXFLAGS += -std=c++11 -pthread
LIBS += -pthread

VAA3DMAINPATH = ../../../../v3d_external/v3d_main

INCLUDEPATH     += /home/yangy/work/local/include
INCLUDEPATH     += /home/yangy/work/local/include/ITK-4.13
INCLUDEPATH     += /home/yangy/work/local/include/pcl-1.8
INCLUDEPATH     += /home/yangy/work/local/include/eigen3

INCLUDEPATH	+= $$VAA3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$VAA3DMAINPATH/neuron_editing
INCLUDEPATH     += $$VAA3DMAINPATH/common_lib/include/boost/graph
INCLUDEPATH     += $$VAA3DMAINPATH/common_lib/include

INCLUDEPATH     += ./3rdparty

HEADERS	+= img.h
HEADERS	+= neuronrecon_plugin.h
HEADERS	+= neuronrecon_func.h
HEADERS	+= neuronrecon.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/basic_4dimage.h
HEADERS += $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.h
HEADERS += $$VAA3DMAINPATH/neuron_editing/neuron_sim_scores.h
HEADERS += $$VAA3DMAINPATH/common_lib/include/boost/graph/prim_minimum_spanning_tree.hpp
HEADERS += $$VAA3DMAINPATH/common_lib/include/boost/graph/kruskal_min_spanning_tree.hpp
HEADERS += $$VAA3DMAINPATH/common_lib/include/boost/graph/adjacency_list.hpp
HEADERS += $$VAA3DMAINPATH/common_lib/include/boost/config.hpp
HEADERS	+= $$VAA3DMAINPATH/neuron_editing/v_neuronswc.h

SOURCES	+= img.cxx
SOURCES	+= neuronrecon_plugin.cpp
SOURCES	+= neuronrecon_func.cpp
SOURCES	+= neuronrecon.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DMAINPATH/neuron_editing/v_neuronswc.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/basic_4dimage.cpp

macx{
    LIBS += -L$$VAA3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$VAA3DMAINPATH/jba/c++ -lv3dnewmat
}

unix:!macx {
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
    LIBS += -L$$VAA3DMAINPATH/jba/c++ -lv3dnewmat
}

LIBS += -lOpenCL
LIBS += -lITKIOTIFF-4.13 -lITKGPUCommon-4.13 -lITKGPUAnisotropicSmoothing-4.13
LIBS += -lpcl_kdtree -lpcl_common -lflann

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

LIBS += /local1/work/tools/v3d_external/bin/plugins/neuron_tracing/ultratracer/libultratracer.so

TARGET	= $$qtLibraryTarget(neurontree_construct)
DESTDIR	= $$VAA3DMAINPATH/../bin/plugins/neuron_utilities/neurontree_construct/
