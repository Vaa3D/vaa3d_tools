#add this pro file by Hanchuan Peng, 2011-02-10
# for nuclei cell segmentation
# by Fuhui Long

TEMPLATE      = lib

CONFIG       += qt plugin warn_off
#CONFIG += debug #this can be turned on for debug even vaa3d is release version for linux
#CONFIG       += x86_64

V3DMAINDIR = ../../../v3d_main
SRCDIR = src

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

HEADERS      += $$V3DMAINDIR/basic_c_fun/stackutil.h 
HEADERS      += $$V3DMAINDIR/basic_c_fun/img_definition.h 
HEADERS      += $$V3DMAINDIR/basic_c_fun/volimg_proc.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/basic_memory.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$SRCDIR/FL_filter3D.h 
HEADERS      += $$SRCDIR/FL_main_brainseg.h 
HEADERS      += $$SRCDIR/FL_regionProps.h 
HEADERS      += $$SRCDIR/FL_bwlabel2D3D.h 
HEADERS      += $$SRCDIR/FL_morphology.h 
HEADERS      += $$SRCDIR/FL_bwdist.h 
HEADERS      += $$SRCDIR/FL_cellSegmentation3D.h
HEADERS      += $$SRCDIR/FL_gvfCellSeg.h
HEADERS      += $$SRCDIR/FL_main_brainseg.h
HEADERS      += $$SRCDIR/FL_downSample3D.h
HEADERS      += $$SRCDIR/FL_defType.h 
HEADERS      += $$SRCDIR/FL_volimgProcLib.h 
HEADERS      += $$SRCDIR/FL_cellSegmentation3D.h 
HEADERS      += $$SRCDIR/FL_cellSegmentation3D.cpp
HEADERS      += $$SRCDIR/FL_sort.h 
HEADERS      += $$SRCDIR/FL_watershed_vs.h 
HEADERS      += $$SRCDIR/FL_distanceTransform3D.h
HEADERS      += $$SRCDIR/FL_threshold.h
HEADERS      += $$SRCDIR/FL_adaptiveThreshold3D.h 
HEADERS      += $$SRCDIR/FL_accessory.h
HEADERS      += $$SRCDIR/dialog_watershed_para.h 
HEADERS      += $$SRCDIR/plugin_FL_cellseg.h

SOURCES      += $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp 
SOURCES      += $$V3DMAINDIR/basic_c_fun/mg_utilities.cpp 
SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp 
SOURCES      += $$SRCDIR/FL_morphology.cpp 
SOURCES      += $$SRCDIR/FL_evolve.cpp 
SOURCES      += $$SRCDIR/dialog_watershed_para.cpp 
SOURCES      += $$SRCDIR/plugin_FL_cellseg.cpp 


#SOURCES      += FL_main_gvfCellSeg.cpp
#SOURCES      += FL_main_brainseg.cpp
#SOURCES      += FL_main_cellseg.cpp

FORMS += $$SRCDIR/FL_watershedSegPara.ui 
#FORMS += FL_levelsetSegPara.ui 


LIBS         += -lm -L$$V3DMAINDIR/common_lib/lib -lv3dtiff

TARGET        = $$qtLibraryTarget(plugin_cellseg)
DESTDIR       = ../../v3d/plugins/image_segmentation/Cell_Segmentation
DESTDIR       = $$V3DMAINDIR/../bin/plugins/image_segmentation/Cell_Segmentation

