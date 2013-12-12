#CMake file for tbar src

message ("exe: " ${BUILDEM_READY})

if (BUILDEM_READY)
  message ("Building executable: skeletonize")
  SET(GuiDir ${BUILDEM_DIR}/include/neurolabi/neulib_cpp)
  SET(NeurolabiLibDir ${BUILDEM_DIR}/lib)
else (BUILDEM_READY)
  if (NOT BUILDEM_USED)
    message ("Building executable without buildem: skeletonize")

    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG_")

    SET(NeurolabiDir ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
    SET(GuiDir ${NeurolabiDir}/gui)
    SET(NeurolabiLibDir ${NeurolabiDir}/c/lib)
  endif (NOT BUILDEM_USED)
endif(BUILDEM_READY)

SET(ExternalSource ${GuiDir}/zpoint.cpp ${GuiDir}/zobject3darray.cpp 
  ${GuiDir}/zobject3d.cpp
  ${GuiDir}/zstring.cpp ${GuiDir}/zargumentprocessor.cpp
  ${GuiDir}/zkmeansclustering.cpp ${GuiDir}/zdoublevector.cpp
  ${GuiDir}/zrandomgenerator.cpp ${GuiDir}/swctreenode.cpp
  ${GuiDir}/zswctree.cpp ${GuiDir}/zswcbranch.cpp ${GuiDir}/zswcpath.cpp
  ${GuiDir}/flyem/zsynapselocationmatcher.cpp
  ${GuiDir}/zswcforest.cpp ${GuiDir}/flyem/zsynapseannotation.cpp
  ${GuiDir}/flyem/zsynapseannotationarray.cpp
  ${GuiDir}/zjsonarray.cpp ${GuiDir}/zjsonvalue.cpp ${GuiDir}/zjsonobject.cpp
  ${GuiDir}/zvaa3dmarker.cpp ${GuiDir}/flyem/zsynapseannotationmetadata.cpp
  ${GuiDir}/zfilelist.cpp ${GuiDir}/zvaa3dapo.cpp
  ${GuiDir}/flyem/zsynapseannotationanalyzer.cpp
  ${GuiDir}/flyem/zsynapselocationmetric.cpp ${GuiDir}/zinttree.cpp
  ${GuiDir}/zswcpath.cpp ${GuiDir}/zjsonparser.cpp ${GuiDir}/zfiletype.cpp
  ${GuiDir}/zcuboid.cpp
  ${GuiDir}/zweightedpoint.cpp ${GuiDir}/zweightedpointarray.cpp
  ${GuiDir}/c_stack.cpp ${GuiDir}/zstackfile.cpp
  ${GuiDir}/zhdf5reader.cpp ${GuiDir}/mylib/array.cpp
  ${GuiDir}/mylib/mylib.c ${GuiDir}/mylib/utilities.cpp ${GuiDir}/zgraph.cpp
  ${GuiDir}/zxmldoc.cpp
  ${GuiDir}/zstack.cxx ${GuiDir}/zsinglechannelstack.cpp
  )

if (BUILDEM_READY)
  message ("Building executable: psd_compare")

  SET(GuiDir ${BUILDEM_DIR}/include/neurolabi/neulib_cpp)
  SET(NeurolabiLibDir ${BUILDEM_DIR}/lib)
  MESSAGE(${GuiDir})


  message(${GuiDir})
  EXECUTE_PROCESS(COMMAND mkdir -p ${GuiDir}/flyem)
  foreach (file ${ExternalSource})
    ADD_CUSTOM_COMMAND(OUTPUT ${file} COMMAND touch ${file})
  endforeach(file)

  LINK_DIRECTORIES (${BUILDEM_DIR}/lib)
  INCLUDE_DIRECTORIES(${BUILDEM_DIR}/include ${BUILDEM_DIR}/include/neurolabi
    ${GuiDir})

  ADD_EXECUTABLE (psd_compare psd_compare.cpp ${ExternalSource})

  SET(jsonLib jansson)
  SET(neuLib neurolabi_jansson)

  TARGET_LINK_LIBRARIES(psd_compare ${neuLib} ${jsonLib})

  MESSAGE(STATUS "Library path " ${CMAKE_LIBRARY_PATH} ${CMAKE_INCLUDE_PATH})
  MESSAGE(STATUS "jsonLib " ${jsonLib})
  MESSAGE(STATUS "neuLib " ${neuLib})
else (BUILDEM_READY)
  if (NOT BUILDEM_USED)
    message ("Building executable without buildem: psd_compare")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG_")

    SET(NeurolabiDir ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
    SET(GuiDir ${NeurolabiDir}/gui)
    SET(NeurolabiLibDir ${NeurolabiDir}/c/lib)

    LINK_DIRECTORIES (${NeurolabiLibDir})
    INCLUDE_DIRECTORIES(${NeurolabiDir}/c ${NeurolabiDir}/c/include
      ${NeurolabiDir}/lib/genelib/src ${GuiDir}  /usr/include/libxml2)

    ADD_EXECUTABLE (psd_compare psd_compare.cpp ${ExternalSource})

    FIND_LIBRARY(jsonLib jansson)
    SET(neuLib neurolabi_debug)
    FIND_LIBRARY(xmlLib xml2)

    TARGET_LINK_LIBRARIES(psd_compare ${neuLib} ${jsonLib} ${xmlLib})

    MESSAGE(STATUS "Library path " ${CMAKE_LIBRARY_PATH} ${CMAKE_INCLUDE_PATH})
    MESSAGE(STATUS "jsonLib " ${jsonLib})
    MESSAGE(STATUS "neuLib " ${neuLib})
  endif(NOT BUILDEM_USED)
endif(BUILDEM_READY)



