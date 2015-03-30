
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
# 64 bit AMD Linux requires -fPIC flag for linking static libraries --CMB
if(UNIX AND NOT APPLE)
  if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
	  set_target_properties(V3DITKCommon PROPERTIES COMPILE_FLAGS -fPIC)
  endif()
endif()
TARGET_LINK_LIBRARIES(V3DITKCommon V3DITKCommon
	${ITK_LIBRARIES} V3DInterface V3DITKCommon ${QT_LIBRARIES})

ADD_EXECUTABLE(V3DITKCommonTest CommonTests.cxx)
TARGET_LINK_LIBRARIES(V3DITKCommonTest V3DITKCommon
	${ITK_LIBRARIES} V3DInterface V3DITKCommon ${QT_LIBRARIES})

