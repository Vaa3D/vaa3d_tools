#
# Install libneurolabi from source
#

if (NOT libneurolabi_NAME)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

include (ExternalProject)
include (ExternalSource)
include (BuildSupport)

external_git_repo(libneurolabi
  HEAD
  $ENV{HOME}/Work/neutube/.git
  )

message ("Installing ${libneurolabi_NAME} into FlyEM build area: ${BUILDEM_DIR} ...")
ExternalProject_Add(${libneurolabi_NAME}
    PREFIX              ${BUILDEM_DIR}
    GIT_REPOSITORY      ${libneurolabi_URL}
    CONFIGURE_COMMAND   git checkout flyem3d
    BUILD_COMMAND       ${BUILDEM_ENV_STRING} ${libneurolabi_SRC_DIR}/neurolabi/update_library --enable-shared
    BUILD_IN_SOURCE     1
    INSTALL_COMMAND     ${BUILDEM_ENV_STRING} ${libneurolabi_SRC_DIR}/neurolabi/install_library ${BUILDEM_DIR}
)
endif (NOT libneurolabi_NAME)
