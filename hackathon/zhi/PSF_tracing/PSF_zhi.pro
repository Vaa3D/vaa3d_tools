
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external/v3d_main
INCLUDEPATH	+= $$VAA3DPATH/v3d $$VAA3DPATH/basic_c_fun $$VAA3DPATH/common_lib/include
#INCLUDEPATH     += /local1/Software/MATLAB/R2013a/extern/include
INCLUDEPATH     += functions

#LIBS += -L/local1/Software/MATLAB/R2013a/bin/glnxa64/ -lmat


HEADERS	+= PSF_zhi_plugin.h
SOURCES	+= PSF_zhi_plugin.cpp
SOURCES	+= $$VAA3DPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/basic_c_fun/basic_surf_objs.cpp


SOURCES	+= functions/PreProcessDataImage.cpp
SOURCES	+= functions/Create3DLookUpTable.cpp
SOURCES	+= functions/Parse_Input.cpp
SOURCES += functions/FunctionsForMainCode.cpp
SOURCES += functions/EigenDec_3D.cpp
SOURCES += functions/Gamma.cpp
SOURCES += functions/MatrixMultiplication.cpp
SOURCES += functions/KernelDensityEstimation.cpp
SOURCES += functions/ML_Divide.cpp
SOURCES += functions/EigenDec_2D.cpp
SOURCES += functions/extractTree.cpp

TARGET	= $$qtLibraryTarget(PSF_zhi)
DESTDIR	= ../../../../v3d_external/bin/plugins/PSF_zhi/
