
SET(PLUGIN_GROUP_NAME "RegistrationPlugin")

SET(Qt_SRCS 
    RegistrationDlg.cxx    
    )

QT4_WRAP_CPP(QT_MOC_SRCS
    RegistrationDlg.h
    )
ADD_LIBRARY(V3DITKRegistration ${Qt_SRCS} ${QT_MOC_SRCS})
TARGET_LINK_LIBRARIES(V3DITKRegistration
 ITKIO  ITKNumerics ITKStatistics ITKAlgorithms V3DInterface V3DITKCommon ${QT_LIBRARIES} )

CONFIGURE_V3D_PLUGIN(Registration)
ADD_SUBDIRECTORY(FilterPlugin)


