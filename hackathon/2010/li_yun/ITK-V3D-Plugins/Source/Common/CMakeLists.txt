
SET(QtITK_SRCS
  V3DITKPluginBaseSingleImage.cxx
  V3DITKGenericDialog.cxx
  V3DITKProgressDialog.cxx
  V3DITKImageSelectionDialog.cxx
  itkEventToQtSignalTranslator.cxx
  )

QT4_WRAP_CPP(QT_MOC_SRCS
  V3DITKPluginBaseSingleImage.h
  V3DITKGenericDialog.h
  V3DITKProgressDialog.h
  V3DITKImageSelectionDialog.h
  itkEventToQtSignalTranslator.h
  )

ADD_LIBRARY(V3DITKCommon ${QtITK_SRCS} ${QT_MOC_SRCS})

ADD_EXECUTABLE(V3DITKCommonTest CommonTests.cxx)
TARGET_LINK_LIBRARIES(V3DITKCommonTest V3DITKCommon
 ITKIO  ITKNumerics ITKStatistics ITKAlgorithms V3DInterface V3DITKCommon ${QT_LIBRARIES} )
