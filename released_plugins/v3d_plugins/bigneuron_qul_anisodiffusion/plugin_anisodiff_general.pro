# 3D anisotropic diffusion plugin for neuron reconstruction
# by Lei Qu
# 2015-03-16

TEMPLATE	= lib
CONFIG		+= qt plugin warn_off
#CONFIG		+= x86_64

SRCFOLDER = ./src/
V3DMAINPATH   = $$SRCFOLDER/../../../../v3d_main

INCLUDEPATH  += $$V3DMAINPATH/common_lib/include
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun
INCLUDEPATH  += $$V3DMAINPATH/v3d

unix {
	LIBS += -L$$V3DMAINPATH/common_lib/lib -lv3dtiff
}

win32 {
	LIBS 	+= -L$$V3DMAINPATH/common_lib/winlib64 -llibtiff 
}

HEADERS      += $$V3DMAINPATH/basic_c_fun/v3d_message.h
HEADERS	     += $$V3DMAINPATH/basic_c_fun/stackutil.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/mg_image_lib.h
HEADERS      += $$V3DMAINPATH/basic_c_fun/mg_utilities.h
HEADERS      += $$SRCFOLDER/q_AnisoDiff3D.h
HEADERS      += $$SRCFOLDER/q_derivatives3D.h
HEADERS      += $$SRCFOLDER/EigenDecomposition3.h
HEADERS      += $$SRCFOLDER/q_EigenVectors3D.h
HEADERS      += $$SRCFOLDER/q_imgaussian3D.h
HEADERS      += $$SRCFOLDER/plugin_anisodiff.h

SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES      += $$V3DMAINPATH//basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES      += $$SRCFOLDER/q_AnisoDiff3D.cpp
SOURCES      += $$SRCFOLDER/q_derivatives3D.cpp
SOURCES      += $$SRCFOLDER/EigenDecomposition3.cpp
SOURCES      += $$SRCFOLDER/q_EigenVectors3D.cpp
SOURCES      += $$SRCFOLDER/q_imgaussian3D.cpp
SOURCES      += $$SRCFOLDER/plugin_anisodiff.cpp


TARGET	= $$qtLibraryTarget(anisodiff)
#DESTDIR	= ../../v3d/plugins/anisodiff/anisodiff_general/
DESTDIR	= $$V3DMAINPATH/../bin/plugins/image_filters/anisotropic_filter/anisodiff_general/
