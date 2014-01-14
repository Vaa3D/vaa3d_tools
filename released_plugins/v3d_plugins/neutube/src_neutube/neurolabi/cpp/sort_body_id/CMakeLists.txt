cmake_minimum_required(VERSION 2.8)

PROJECT(EXTRACT_BOUNDED_NEURON)

include (ExternalProject)

set(BUILDEM_DIR "None" CACHE TYPE STRING)

if (${BUILDEM_DIR} STREQUAL "None")
  message (FATAL_ERROR "ERROR: FlyEM build directory (for all downloads & builds) should be specified via -DBUILDEM_DIR=<path> on cmake command line.")
endif ()

message ("FlyEM downloads and builds will be placed here: ${BUILDEM_DIR}")

############################################################################

############################################################################
# Download and install buildem, if it isn't already in BUILDEM_DIR.
set (BUILDEM_REPO_DIR ${BUILDEM_DIR}/src/buildem)
set (BUILDEM_READY TRUE)
if (NOT EXISTS ${BUILDEM_REPO_DIR}/python.cmake)
  set (BUILDEM_USED TRUE)
  message ("Installing buildem repo...")
  set (BUILDEM_READY FALSE)
  ExternalProject_Add(buildem
    PREFIX              ${BUILDEM_DIR}
    GIT_REPOSITORY http://github.com/janelia-flyem/buildem.git
    #GIT_REPOSITORY $ENV{HOME}/tmp/buildem/.git
    UPDATE_COMMAND      ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND "" 
    BUILD_COMMAND ""
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
    )
  message ("\n**********************************************************\n")
  message ("\nAfter running make, you must re-run the cmake command once")
  message ("buildem has been downloaded!\n")
  message ("\n***********************************************************\n")
else ()
  set (BUILDEM_USED TRUE)
  ############################################################################
  #Use modules  from  the  downloaded  buildem 
  set (CMAKE_MODULE_PATH ${BUILDEM_REPO_DIR})
  message("Using cmake modules from ${BUILDEM_REPO_DIR}")

  # Download and compile dependencies
  include(libjansson)
  SET(UPDATE_NEULIB ON CACHE BOOL "Update neuLib")
  if (UPDATE_NEULIB)
    include (neulib_jansson)
  endif(UPDATE_NEULIB)

  # Install tbar -- actual build commands would replace the placeholder below.
  # Note the auto-generated APP_DEPENDENCIES variable that holds all required targets.
  #add_custom_target (mapbody ALL DEPENDS ${APP_DEPENDENCIES} COMMENT "mapbody built")

  ############################################################################
endif()


ADD_SUBDIRECTORY(src bin)


