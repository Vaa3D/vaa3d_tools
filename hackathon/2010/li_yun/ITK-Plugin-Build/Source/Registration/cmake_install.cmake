# Install script for directory: /home/liyun/V3D/2010_hackathon/li_yun/ITK-V3D-Plugins/Source/Registration

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
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationAffine/libRegistrationAffine.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationAffine/libRegistrationAffine.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationAffine/libRegistrationAffine.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationAffine" TYPE SHARED_LIBRARY PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ FILES "/home/liyun/V3D/2010_hackathon/li_yun/ITK-Plugin-Build/bin/libRegistrationAffine.so")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationAffine/libRegistrationAffine.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationAffine/libRegistrationAffine.so")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationAffine/libRegistrationAffine.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationAffine/libRegistrationAffine.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/DemonsRegistration/libDemonsRegistration.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/DemonsRegistration/libDemonsRegistration.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/DemonsRegistration/libDemonsRegistration.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "/home/liyun/V3D/v3d/plugins/ITK/Registration/DemonsRegistration" TYPE SHARED_LIBRARY PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ FILES "/home/liyun/V3D/2010_hackathon/li_yun/ITK-Plugin-Build/bin/libDemonsRegistration.so")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/DemonsRegistration/libDemonsRegistration.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/DemonsRegistration/libDemonsRegistration.so")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/DemonsRegistration/libDemonsRegistration.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/DemonsRegistration/libDemonsRegistration.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_2D/libRegistrationBSpline_2D.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_2D/libRegistrationBSpline_2D.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_2D/libRegistrationBSpline_2D.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_2D" TYPE SHARED_LIBRARY PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ FILES "/home/liyun/V3D/2010_hackathon/li_yun/ITK-Plugin-Build/bin/libRegistrationBSpline_2D.so")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_2D/libRegistrationBSpline_2D.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_2D/libRegistrationBSpline_2D.so")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_2D/libRegistrationBSpline_2D.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_2D/libRegistrationBSpline_2D.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_3D/libRegistrationBSpline_3D.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_3D/libRegistrationBSpline_3D.so")
    FILE(RPATH_CHECK
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_3D/libRegistrationBSpline_3D.so"
         RPATH "")
  ENDIF()
  FILE(INSTALL DESTINATION "/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_3D" TYPE SHARED_LIBRARY PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ FILES "/home/liyun/V3D/2010_hackathon/li_yun/ITK-Plugin-Build/bin/libRegistrationBSpline_3D.so")
  IF(EXISTS "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_3D/libRegistrationBSpline_3D.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_3D/libRegistrationBSpline_3D.so")
    FILE(RPATH_REMOVE
         FILE "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_3D/libRegistrationBSpline_3D.so")
    IF(CMAKE_INSTALL_DO_STRIP)
      EXECUTE_PROCESS(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}/home/liyun/V3D/v3d/plugins/ITK/Registration/RegistrationBSpline_3D/libRegistrationBSpline_3D.so")
    ENDIF(CMAKE_INSTALL_DO_STRIP)
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "RuntimeLibraries")

