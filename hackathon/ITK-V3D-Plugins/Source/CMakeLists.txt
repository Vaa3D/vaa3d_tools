
INCLUDE_DIRECTORIES(
  Common
  ${V3D_BASIC_C_FUN_SOURCE_DIR}
  )

ADD_SUBDIRECTORY(
  ${V3D_BASIC_C_FUN_SOURCE_DIR}
  ${V3D_BASIC_C_FUN_BINARY_DIR}
  )

# FIXED by Alessandro on 2014-12-05. Including the CMakefile.txt in mylib_tiff to produce the missing mylib_tiff library. 
if (NOT WIN32)
	ADD_SUBDIRECTORY(${V3D_BASIC_C_FUN_SOURCE_DIR}/../common_lib/src_packages/mylib_tiff "${CMAKE_CURRENT_BINARY_DIR}/mylib_tiff")
endif()

ADD_SUBDIRECTORY(Common)
ADD_SUBDIRECTORY(Superplugin_new_v1)
ADD_SUBDIRECTORY(Superplugin_new_v2)
ADD_SUBDIRECTORY(Superplugin)
ADD_SUBDIRECTORY(Arithmetic)
ADD_SUBDIRECTORY(BinaryLogic)
ADD_SUBDIRECTORY(IntensityTransformation)
ADD_SUBDIRECTORY(MathMorphology)
ADD_SUBDIRECTORY(Smoothing)
ADD_SUBDIRECTORY(Thresholding)
ADD_SUBDIRECTORY(EdgeDetection)
ADD_SUBDIRECTORY(LevelSets)
ADD_SUBDIRECTORY(RegionGrowing)
ADD_SUBDIRECTORY(Classification)
ADD_SUBDIRECTORY(ImageObjects)
ADD_SUBDIRECTORY(Watershed)
ADD_SUBDIRECTORY(DistanceMaps)
ADD_SUBDIRECTORY(Plugin2Call)
ADD_SUBDIRECTORY(PureITKPlugin)

