
SET(Qt_SRCS
  Dialog.cpp
  pages.cpp	
  qrc_Icons.cpp
  )

QT4_WRAP_CPP(QT_MOC_SRCS
  dialog.h
  pages.h
  )

ADD_LIBRARY(SuperPluginDlg ${Qt_SRCS} ${QT_MOC_SRCS})
TARGET_LINK_LIBRARIES(SuperPluginDlg 
 ITKIO  ITKNumerics ITKStatistics ITKAlgorithms V3DInterface V3DITKCommon ${QT_LIBRARIES} )

