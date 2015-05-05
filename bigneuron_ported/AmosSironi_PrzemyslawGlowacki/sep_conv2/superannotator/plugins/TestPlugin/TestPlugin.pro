TEMPLATE = lib
INCLUDEPATH += ../../

HEADERS += \
    TestPlugin.h \
    ../PluginBase.h

SOURCES += \
    TestPlugin.cpp

# ITK PATH
include(../../customUserDefs.inc)

ITKPATH_BUILD = $$ITKPATH/build

#### ITK STUFF

INCLUDEPATH += $$ITKPATH/Code/Review
INCLUDEPATH += $$ITKPATH_BUILD/Code/Review

INCLUDEPATH += $$ITKPATH/Utilities/gdcm/src
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/gdcm/src

INCLUDEPATH += $$ITKPATH/Utilities/gdcm
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/gdcm

INCLUDEPATH += $$ITKPATH/Utilities/vxl/core
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/vxl/core

INCLUDEPATH += $$ITKPATH/Utilities/vxl/vcl
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/vxl/vcl

INCLUDEPATH += $$ITKPATH/Utilities/vxl/v3p/netlib
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/vxl/v3p/netlib

INCLUDEPATH += $$ITKPATH/Utilities/vxl/core
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/vxl/core

INCLUDEPATH += $$ITKPATH/Utilities/vxl/vcl
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/vxl/vcl

INCLUDEPATH += $$ITKPATH/Utilities/vxl/v3p/netlib
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/vxl/v3p/netlib

INCLUDEPATH += $$ITKPATH/Code/Numerics/Statistics
INCLUDEPATH += $$ITKPATH_BUILD/Code/Numerics/Statistics

INCLUDEPATH += $$ITKPATH/Utilities
INCLUDEPATH += $$ITKPATH_BUILD/Utilities

INCLUDEPATH += $$ITKPATH/Utilities/itkExtHdrs
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/itkExtHdrs

INCLUDEPATH += $$ITKPATH/Utilities/nifti/znzlib
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/nifti/znzlib

INCLUDEPATH += $$ITKPATH/Utilities/nifti/niftilib
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/nifti/niftilib

INCLUDEPATH += $$ITKPATH/Utilities/expat
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/expat

INCLUDEPATH += $$ITKPATH/Utilities/DICOMParser
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/DICOMParser

INCLUDEPATH += $$ITKPATH/Utilities/NrrdIO
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/NrrdIO

INCLUDEPATH += $$ITKPATH/Utilities/MetaIO
INCLUDEPATH += $$ITKPATH_BUILD/Utilities/MetaIO

INCLUDEPATH += $$ITKPATH/Code/SpatialObject
INCLUDEPATH += $$ITKPATH_BUILD/Code/SpatialObject

INCLUDEPATH += $$ITKPATH/Code/Numerics/NeuralNetworks
INCLUDEPATH += $$ITKPATH_BUILD/Code/Numerics/NeuralNetworks

INCLUDEPATH += $$ITKPATH/Code/Numerics/FEM
INCLUDEPATH += $$ITKPATH_BUILD/Code/Numerics/FEM

INCLUDEPATH += $$ITKPATH/Code/IO
INCLUDEPATH += $$ITKPATH_BUILD/Code/IO

INCLUDEPATH += $$ITKPATH/Code/Numerics
INCLUDEPATH += $$ITKPATH_BUILD/Code/Numerics

INCLUDEPATH += $$ITKPATH/Code/Common
INCLUDEPATH += $$ITKPATH_BUILD/Code/Common

INCLUDEPATH += $$ITKPATH/Code/BasicFilters
INCLUDEPATH += $$ITKPATH_BUILD/Code/BasicFilters

INCLUDEPATH += $$ITKPATH/Code/Algorithms
INCLUDEPATH += $$ITKPATH_BUILD/Code/Algorithms

INCLUDEPATH += $$ITKPATH/
INCLUDEPATH += $$ITKPATH_BUILD/

LIBS += -L$$ITKPATH_BUILD/bin -lITKIO -lITKStatistics -lITKNrrdIO -litkgdcm -litkjpeg12 -litkjpeg16 -litkopenjpeg -litkpng -litktiff -litkjpeg8 -lITKSpatialObject -lITKMetaIO -lITKDICOMParser -lITKEXPAT -lITKniftiio -lITKznz -litkzlib -lITKCommon -litksys -litkvnl_inst -litkvnl_algo -litkvnl -litkvcl -litkv3p_lsqr -lpthread -lm -ldl -litkNetlibSlatec -litkv3p_netlib
LIBS += -luuid
