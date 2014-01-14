#CMake file for tbar src

message ("exe: " ${BUILDEM_READY})
if (BUILDEM_READY)
  message ("Building executable: sort_body_id")
  SET(GuiDir ${BUILDEM_DIR}/include/neurolabi/neulib_cpp)
  SET(NeurolabiLibDir ${BUILDEM_DIR}/lib)
else (BUILDEM_READY)
  if (NOT BUILDEM_USED)
    message ("Building executable without buildem: sort_body_id")

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG_")

    SET(NeurolabiDir ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
    SET(GuiDir ${NeurolabiDir}/gui)
    SET(NeurolabiLibDir ${NeurolabiDir}/c/lib)
  endif (NOT BUILDEM_USED)
endif(BUILDEM_READY)

SET(ExternalSource ${GuiDir}/zswctree.cpp
  ${GuiDir}/zspgrowparser.cpp ${GuiDir}/zsegmentmaparray.cpp
  ${GuiDir}/zsuperpixelmap.cpp ${GuiDir}/zsuperpixelmaparray.cpp
  ${GuiDir}/zvoxel.cpp ${GuiDir}/zvoxelarray.cpp ${GuiDir}/zsegmentmap.cpp
  ${GuiDir}/zswcforest.cpp ${GuiDir}/zpoint.cpp ${GuiDir}/zswcbranch.cpp
  ${GuiDir}/zstring.cpp ${GuiDir}/zfilelist.cpp ${GuiDir}/swctreenode.cpp 
  ${GuiDir}/zrandomgenerator.cpp ${GuiDir}/zstack.cxx ${GuiDir}/zswcpath.cpp 
  ${GuiDir}/zstackfile.cpp 
  ${GuiDir}/zsinglechannelstack.cpp ${GuiDir}/c_stack.cpp
  ${GuiDir}/zxmldoc.cpp ${GuiDir}/zfiletype.cpp ${GuiDir}/zjsonobject.cpp
  ${GuiDir}/zjsonvalue.cpp ${GuiDir}/zjsonparser.cpp
  ${GuiDir}/zhdf5reader.cpp ${GuiDir}/mylib/array.cpp
  ${GuiDir}/mylib/mylib.c ${GuiDir}/mylib/utilities.cpp ${GuiDir}/zgraph.cpp
  ${GuiDir}/zcuboid.cpp ${GuiDir}/zweightedpoint.cpp 
  ${GuiDir}/zweightedpointarray.cpp ${GuiDir}/zargumentprocessor.cpp)

message(${GuiDir})

if (BUILDEM_READY)
  EXECUTE_PROCESS(COMMAND mkdir -p ${GuiDir}/flyem)
  EXECUTE_PROCESS(COMMAND mkdir -p ${GuiDir}/mylib)

  foreach (file ${ExternalSource})
    ADD_CUSTOM_COMMAND(OUTPUT ${file} COMMAND touch ${file})
  endforeach(file)

  LINK_DIRECTORIES (${BUILDEM_DIR}/lib)
  INCLUDE_DIRECTORIES(${BUILDEM_DIR}/include ${BUILDEM_DIR}/include/neurolabi
    ${GuiDir})

  ADD_EXECUTABLE (sort_body_id sort_body_id.cpp ${ExternalSource})

  SET(jsonLib jansson)
  SET(neuLib neurolabi_jansson)

  TARGET_LINK_LIBRARIES(sort_body_id ${neuLib} ${jsonLib})

  MESSAGE(STATUS "Library path " ${CMAKE_LIBRARY_PATH} ${CMAKE_INCLUDE_PATH})
  MESSAGE(STATUS "jsonLib " ${jsonLib})
  MESSAGE(STATUS "neuLib " ${neuLib})
else (BUILDEM_READY)
  if (NOT BUILDEM_USED)
    LINK_DIRECTORIES (${NeurolabiLibDir})
    INCLUDE_DIRECTORIES(${NeurolabiDir}/c ${NeurolabiDir}/c/include
      ${NeurolabiDir}/lib/genelib/src ${GuiDir})

    ADD_EXECUTABLE (sort_body_id sort_body_id.cpp ${ExternalSource})

    FIND_LIBRARY(jsonLib jansson)
    SET(neuLib neurolabi_debug)

    TARGET_LINK_LIBRARIES(sort_body_id ${neuLib} ${jsonLib})

    MESSAGE(STATUS "Library path " ${CMAKE_LIBRARY_PATH} ${CMAKE_INCLUDE_PATH})
    MESSAGE(STATUS "jsonLib " ${jsonLib})
    MESSAGE(STATUS "neuLib " ${neuLib})
  endif(NOT BUILDEM_USED)
endif(BUILDEM_READY)



