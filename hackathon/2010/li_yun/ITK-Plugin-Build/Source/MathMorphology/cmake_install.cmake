# Install script for directory: /home/liyun/V3D/2010_hackathon/li_yun/ITK-V3D-Plugins/Source/MathMorphology

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
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/MathMorphology/Erode/libErode.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/MathMorphology/Erode/libErode.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/MathMorphology/Erode/libErode.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "/home/liyun/V3D/v3d/plugins/ITK/MathMorphology/Erode" TYPE SHARED_LIBRARY PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ FILES "/home/liyun/V3D/2010_hackathon/li_yun/ITK-Plugin-Build/bin/libErode.so")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/MathMorphology/Erode/libErode.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/MathMorphology/Erode/libErode.so")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/MathMorphology/Erode/libErode.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/MathMorphology/Erode/libErode.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")

