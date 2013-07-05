
TEMPLATE      = lib
CONFIG       += qt plugin warn_off

V3DMAINDIR    = ../../../v3d_external/v3d_main/

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun

HEADERS       = $$V3DMAINDIR/basic_c_fun/img_definition.h
HEADERS       = $$V3DMAINDIR/basic_c_fun/v3d_basicdatatype.h
HEADERS       = $$V3DMAINDIR/basic_c_fun/volimg_proc.h
HEADERS      += FL_defType.h
HEADERS      += FL_accessory.h
HEADERS      += FL_neighborhood.h
HEADERS      += FL_unionFind.h
HEADERS      += FL_neighborhoodWalker.h
HEADERS      += FL_bwlabel2D3D.h
HEADERS      += bwlabel.h
HEADERS      += label_object_dialog.h


SOURCES       = $$V3DMAINDIR/basic_c_fun/v3d_message.cpp
SOURCES      += bwlabel.cpp

TARGET        = $$qtLibraryTarget(fast_bwlabel)
DESTDIR       = $$V3DMAINDIR/../v3d/plugins/unfinished/Fast_Label_Image_Objects

