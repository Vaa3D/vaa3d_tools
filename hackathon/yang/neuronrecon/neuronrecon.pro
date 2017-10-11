TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64

VAA3DMAINPATH = ../../../../v3d_external/v3d_main

INCLUDEPATH     += /home/yangy/work/local/include
INCLUDEPATH     += /home/yangy/work/local/include/ITK-4.13
INCLUDEPATH     += /home/yangy/work/local/include/pcl-1.8
INCLUDEPATH     += /home/yangy/work/local/include/eigen3

INCLUDEPATH	+= $$VAA3DMAINPATH/basic_c_fun
INCLUDEPATH	+= $$VAA3DMAINPATH/neuron_editing
INCLUDEPATH     += $$VAA3DMAINPATH/common_lib/include/boost/graph
INCLUDEPATH     += $$VAA3DMAINPATH/common_lib/include/

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

SOURCES	+= neuronrecon_plugin.cpp
SOURCES	+= neuronrecon_func.cpp
SOURCES	+= neuronrecon.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES	+= $$VAA3DMAINPATH/neuron_editing/v_neuronswc.cpp
SOURCES	+= $$VAA3DMAINPATH/basic_c_fun/basic_surf_objs.cpp

LIBS += -lOpenCL
LIBS += /home/yangy/work/local/lib/libITKIOTIFF-4.13.so
LIBS += /home/yangy/work/local/lib/libITKGPUCommon-4.13.so
LIBS += /home/yangy/work/local/lib/libITKGPUAnisotropicSmoothing-4.13.so

LIBS += /local1/work/tools/v3d_external/bin/plugins/neuron_tracing/ultratracer/libultratracer.so

LIBS += /home/yangy/work/local/lib/libpcl_kdtree.so
LIBS += /home/yangy/work/local/lib/libpcl_common.so
LIBS += /home/yangy/work/local/lib/libflann.so
LIBS += /home/yangy/work/local/lib/libboost_system.so

TARGET	= $$qtLibraryTarget(neurontree_construct)
DESTDIR	= $$VAA3DMAINPATH/../bin/plugins/neuron_utilities/neurontree_construct/
