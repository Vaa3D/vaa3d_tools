cmake_minimum_required(VERSION 2.8)

project (neutube)

add_custom_target(
  neutube
  DEPENDS neuTube
  )

add_custom_command(
  OUTPUT neuTube
  COMMAND rm -rf qt && mkdir qt
  COMMAND qmake ${PROJECT_SOURCE_DIR}/gui.pro -r -spec macx-g++ CONFIG+=x86_64 CONFIG+=declarative_debug -o qt/Makefile
  COMMAND echo "make neurolabi" && cd qt && make neurolabi -w
  COMMAND cd qt && make -w
  WORKING_DIRECTORY ${CMAKE_BIN_DIR}
  )

