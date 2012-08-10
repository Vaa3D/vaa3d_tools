
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
#add -fPIC if the system is unix
if (UNIX AND NOT APPLE)
        if ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
                set_target_properties(V3DITKCommon PROPERTIES COMPILE_FLAGS -fPIC)
        endif()
endif()

ADD_EXECUTABLE(V3DITKCommonTest CommonTests.cxx)
#TARGET_LINK_LIBRARIES(V3DITKCommonTest V3DITKCommon
# ITKIO  ITKNumerics ITKStatistics ITKAlgorithms V3DInterface V3DITKCommon ${QT_LIBRARIES} )
#LINK_LIBRARIES(/home/yuping/yp/ITK/ITKv4/lib)
#TARGET_LINK_LIBRARIES(V3DITKCommonTest V3DITKCommon
#	ITKCommon-4.1 ITKIOImageBase-4.1 itkvnl-4.1 ITKStatistics-4.1 V3DInterface V3DITKCommon ${QT_LIBARIES})
TARGET_LINK_LIBRARIES(V3DITKCommonTest V3DITKCommon
	${ITK_LIBRARIES} V3DInterface V3DITKCommon ${QT_LIBRARIES})
