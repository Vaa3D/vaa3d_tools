#CMake file for tbar src

message ("exe: " ${BUILDEM_READY})

if (BUILDEM_READY)
  message ("Building executable: map_body")
  SET(GuiDir ${BUILDEM_DIR}/include/neurolabi/neulib_cpp)
  SET(NeurolabiLibDir ${BUILDEM_DIR}/lib)
else (BUILDEM_READY)
  if (NOT BUILDEM_USED)
    message ("Building executable without buildem: map_body")

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG_")

    SET(NeurolabiDir ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
    SET(GuiDir ${NeurolabiDir}/gui)
    SET(NeurolabiLibDir ${NeurolabiDir}/c/lib)
  endif (NOT BUILDEM_USED)
endif(BUILDEM_READY)

MESSAGE(${GuiDir})
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
  ${GuiDir}/zweightedpointarray.cpp ${GuiDir}/zobject3dscan.cpp 
  ${GuiDir}/zobject3d.cpp ${GuiDir}/zobject3darray.cpp
  ${GuiDir}/zresolution.cpp ${GuiDir}/zprogressable.cpp
  ${GuiDir}/zprogressreporter.cpp ${GuiDir}/zinttree.cpp)


if (BUILDEM_READY)
  EXECUTE_PROCESS(COMMAND mkdir -p ${GuiDir}/flyem)
  EXECUTE_PROCESS(COMMAND mkdir -p ${GuiDir}/mylib)
  foreach (file ${ExternalSource})
    ADD_CUSTOM_COMMAND(OUTPUT ${file} COMMAND touch ${file})
  endforeach(file)

  LINK_DIRECTORIES (${BUILDEM_DIR}/lib)
  INCLUDE_DIRECTORIES(${BUILDEM_DIR}/include ${BUILDEM_DIR}/include/neurolabi
    ${GuiDir})

  SET(pngLib png)
  SET(neuLib neurolabi_png)

  MESSAGE(STATUS "Library path " ${CMAKE_LIBRARY_PATH} ${CMAKE_INCLUDE_PATH})
  MESSAGE(STATUS "pngLib " ${pngLib})
  MESSAGE(STATUS "neuLib " ${neuLib})

  ADD_EXECUTABLE (map_body map_body.cpp ${ExternalSource})

  TARGET_LINK_LIBRARIES(map_body ${neuLib} ${pngLib})

else (BUILDEM_READY)
  if (NOT BUILDEM_USED)
    LINK_DIRECTORIES (${NeurolabiLibDir})
    INCLUDE_DIRECTORIES(${NeurolabiDir}/c ${NeurolabiDir}/c/include
      ${NeurolabiDir}/lib/genelib/src ${GuiDir})

    ADD_EXECUTABLE (map_body map_body.cpp ${ExternalSource})

    FIND_LIBRARY(pngLib png)
    SET(neuLib neurolabi_debug)

    TARGET_LINK_LIBRARIES(map_body ${neuLib} ${pngLib})

    MESSAGE(STATUS "Library path " ${CMAKE_LIBRARY_PATH} ${CMAKE_INCLUDE_PATH})
    MESSAGE(STATUS "pngLib " ${pngLib})
    MESSAGE(STATUS "neuLib " ${neuLib})
  endif(NOT BUILDEM_USED)
endif(BUILDEM_READY)



