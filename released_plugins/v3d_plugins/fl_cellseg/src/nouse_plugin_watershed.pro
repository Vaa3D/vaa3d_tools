
TEMPLATE      = lib
CONFIG       += qt plugin warn_off

V3DMAINDIR    = ../../../../v3d_external/v3d_main/
SRCDIR1 = ./
SRCDIR = $$V3DMAINDIR/cellseg/


INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun

HEADERS       = $$V3DMAINDIR/basic_c_fun/img_definition.h
HEADERS       = $$V3DMAINDIR/basic_c_fun/v3d_basicdatatype.h
HEADERS       = $$V3DMAINDIR/basic_c_fun/volimg_proc.h
HEADERS      += $$SRCDIR/FL_defType.h
HEADERS      += $$SRCDIR/FL_accessory.h
HEADERS      += $$SRCDIR/FL_neighborhood.h
HEADERS      += $$SRCDIR/FL_unionFind.h
HEADERS      += $$SRCDIR/FL_neighborhoodWalker.h
HEADERS      += $$SRCDIR/FL_bwlabel2D3D.h
HEADERS      += $$SRCDIR/FL_watershed_vs.h
HEADERS      += $$SRCDIR1/label_object_dialog.h
HEADERS      += $$SRCDIR1/plugin_watershed.h


SOURCES       = $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
SOURCES      += $$SRCDIR1/plugin_watershed.cpp

TARGET        = $$qtLibraryTarget(plugin_watershed)
DESTDIR       = $$V3DMAINDIR/../bin/plugins/image_segmentation/Watershed_Segment_Image_Objects

