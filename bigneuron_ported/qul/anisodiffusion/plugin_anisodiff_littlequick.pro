# 3D anisotropic diffusion plugin for neuron reconstruction
# by Lei Qu
# 2015-03-16

TEMPLATE	= lib
CONFIG		+= qt plugin warn_off
#CONFIG		+= x86_64

V3DMAINPATH   = ../../../v3d_main

INCLUDEPATH  += $$V3DMAINPATH/common_lib/include
INCLUDEPATH  += $$V3DMAINPATH/basic_c_fun

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
HEADERS      += q_AnisoDiff3D.h
HEADERS      += q_derivatives3D.h
HEADERS      += EigenDecomposition3.h
HEADERS      += q_EigenVectors3D.h
HEADERS      += q_imgaussian3D.h
HEADERS      += plugin_anisodiff_littlequick.h

SOURCES      += $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES      += $$V3DMAINPATH//basic_c_fun/basic_surf_objs.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES      += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES      += q_AnisoDiff3D.cpp
SOURCES      += q_derivatives3D.cpp
SOURCES      += EigenDecomposition3.cpp
SOURCES      += q_EigenVectors3D.cpp
SOURCES      += q_imgaussian3D.cpp
SOURCES      += plugin_anisodiff_littlequick.cpp


TARGET	= $$qtLibraryTarget(anisodiff_littlequick)
#DESTDIR	= ../../v3d/plugins/anisodiff/
DESTDIR	= ../../../bin/plugins/anisodiff/