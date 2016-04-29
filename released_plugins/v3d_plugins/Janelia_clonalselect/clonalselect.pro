
TEMPLATE      = lib
CONFIG       += qt plugin warn_off
#CONFIG       += release x86_64

V3DMAINDIR = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
INCLUDEPATH  += $$V3DMAINDIR/common_lib/include

HEADERS      +=	clonalselect_gui.h
HEADERS      +=	clonalselect_core.h
HEADERS      += clonalselect.h
HEADERS      +=	$$V3DMAINDIR/basic_c_fun/stackutil.h
HEADERS      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp

SOURCES      += clonalselect_gui.cpp
SOURCES      += clonalselect_core.cpp
SOURCES      += clonalselect.cpp
SOURCES      += $$V3DMAINDIR/basic_c_fun/stackutil.cpp
SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_utilities.cpp
SOURCES      +=	$$V3DMAINDIR/basic_c_fun/mg_image_lib.cpp

LIBS         += -lm -L$$V3DMAINDIR/common_lib/lib -lv3dtiff

TARGET        = $$qtLibraryTarget(Janelia_clonalSelect)
DESTDIR       = $$V3DMAINDIR/../bin/plugins/FlyWorkstation_utilities/Janelia_clonalSelect

