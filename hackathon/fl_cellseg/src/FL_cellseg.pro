#add this pro file by Hanchuan Peng, 2011-02-10
# for nuclei cell segmentation
# by Fuhui Long

TEMPLATE      = lib

CONFIG       += qt plugin warn_off
#CONFIG       += x86_64

V3DMAINDIR = ../../../v3d_external/v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

HEADERS      += $$V3DMAINDIR/basic_c_fun/stackutil.h 
HEADERS      += $$V3DMAINDIR/basic_c_fun/img_definition.h 
HEADERS      += $$V3DMAINDIR/basic_c_fun/volimg_proc.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_memory.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += FL_filter3D.h 
HEADERS      += FL_main_brainseg.h 
HEADERS      += FL_regionProps.h 
HEADERS      += FL_bwlabel2D3D.h 
HEADERS      += FL_morphology.h 
HEADERS      += FL_bwdist.h 
HEADERS      += FL_cellSegmentation3D.h
HEADERS      += FL_gvfCellSeg.h
HEADERS      += FL_main_brainseg.h
HEADERS      += FL_downSample3D.h
HEADERS      += FL_defType.h 
HEADERS      += FL_volimgProcLib.h 
HEADERS      += FL_cellSegmentation3D.h 
HEADERS      += FL_cellSegmentation3D.cpp
HEADERS      += FL_sort.h 
HEADERS      += FL_watershed_vs.h 
HEADERS      += FL_distanceTransform3D.h
HEADERS      += FL_threshold.h
HEADERS      += FL_adaptiveThreshold3D.h 
HEADERS      += FL_accessory.h
HEADERS      += dialog_watershed_para.h 
HEADERS      += plugin_FL_cellseg.h

SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp 
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_utilities.cpp 
SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp 
SOURCES      += FL_morphology.cpp 
SOURCES      += FL_evolve.cpp 
SOURCES      += dialog_watershed_para.cpp 
SOURCES      += plugin_FL_cellseg.cpp 


#SOURCES      += FL_main_gvfCellSeg.cpp
#SOURCES      += FL_main_brainseg.cpp
#SOURCES      += FL_main_cellseg.cpp

FORMS += FL_watershedSegPara.ui 
#FORMS += FL_levelsetSegPara.ui 


LIBS         += -lm -L$$V3DMAINDIR/common_lib/lib -lv3dtiff

TARGET        = $$qtLibraryTarget(plugin_FL_cellseg)
DESTDIR       = $$V3DMAINDIR/../v3d/plugins/FL_cellseg

