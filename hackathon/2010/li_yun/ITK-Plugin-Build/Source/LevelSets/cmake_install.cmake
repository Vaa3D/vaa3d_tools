# Install script for directory: /home/liyun/V3D/2010_hackathon/li_yun/ITK-V3D-Plugins/Source/LevelSets

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarching/libITKFastMarching.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarching/libITKFastMarching.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarching/libITKFastMarching.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarching" TYPE SHARED_LIBRARY PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ FILES "/home/liyun/V3D/2010_hackathon/li_yun/ITK-Plugin-Build/bin/libITKFastMarching.so")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarching/libITKFastMarching.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarching/libITKFastMarching.so")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarching/libITKFastMarching.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarching/libITKFastMarching.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarchingPlus/libITKFastMarchingPlus.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarchingPlus/libITKFastMarchingPlus.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarchingPlus/libITKFastMarchingPlus.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarchingPlus" TYPE SHARED_LIBRARY PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ FILES "/home/liyun/V3D/2010_hackathon/li_yun/ITK-Plugin-Build/bin/libITKFastMarchingPlus.so")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarchingPlus/libITKFastMarchingPlus.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarchingPlus/libITKFastMarchingPlus.so")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarchingPlus/libITKFastMarchingPlus.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKFastMarchingPlus/libITKFastMarchingPlus.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKShapeDetection/libITKShapeDetection.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKShapeDetection/libITKShapeDetection.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKShapeDetection/libITKShapeDetection.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKShapeDetection" TYPE SHARED_LIBRARY PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ FILES "/home/liyun/V3D/2010_hackathon/li_yun/ITK-Plugin-Build/bin/libITKShapeDetection.so")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKShapeDetection/libITKShapeDetection.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKShapeDetection/libITKShapeDetection.so")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKShapeDetection/libITKShapeDetection.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKShapeDetection/libITKShapeDetection.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKGeodesicActiveContour/libITKGeodesicActiveContour.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKGeodesicActiveContour/libITKGeodesicActiveContour.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKGeodesicActiveContour/libITKGeodesicActiveContour.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKGeodesicActiveContour" TYPE SHARED_LIBRARY PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ FILES "/home/liyun/V3D/2010_hackathon/li_yun/ITK-Plugin-Build/bin/libITKGeodesicActiveContour.so")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKGeodesicActiveContour/libITKGeodesicActiveContour.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKGeodesicActiveContour/libITKGeodesicActiveContour.so")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKGeodesicActiveContour/libITKGeodesicActiveContour.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKGeodesicActiveContour/libITKGeodesicActiveContour.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKThresholdSegmentation/libITKThresholdSegmentation.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKThresholdSegmentation/libITKThresholdSegmentation.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKThresholdSegmentation/libITKThresholdSegmentation.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKThresholdSegmentation" TYPE SHARED_LIBRARY PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ FILES "/home/liyun/V3D/2010_hackathon/li_yun/ITK-Plugin-Build/bin/libITKThresholdSegmentation.so")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKThresholdSegmentation/libITKThresholdSegmentation.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKThresholdSegmentation/libITKThresholdSegmentation.so")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKThresholdSegmentation/libITKThresholdSegmentation.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKThresholdSegmentation/libITKThresholdSegmentation.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKCannySegmentation/libITKCannySegmentation.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKCannySegmentation/libITKCannySegmentation.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKCannySegmentation/libITKCannySegmentation.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKCannySegmentation" TYPE SHARED_LIBRARY PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ FILES "/home/liyun/V3D/2010_hackathon/li_yun/ITK-Plugin-Build/bin/libITKCannySegmentation.so")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKCannySegmentation/libITKCannySegmentation.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKCannySegmentation/libITKCannySegmentation.so")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKCannySegmentation/libITKCannySegmentation.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/LevelSets/ITKCannySegmentation/libITKCannySegmentation.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")

