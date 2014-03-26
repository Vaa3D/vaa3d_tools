#CMake file for tbar src

message ("exe: " ${BUILDEM_READY})
if (BUILDEM_READY)
  message ("Building executable: Tbar_apply_sa")

  SET(GuiDir ${BUILDEM_DIR}/include/neurolabi/neulib_cpp)
  SET(NeurolabiLibDir ${BUILDEM_DIR}/lib)
  MESSAGE(${GuiDir})

  SET(ExternalSource ${GuiDir}/zpoint.cpp ${GuiDir}/zobject3darray.cpp 
    ${GuiDir}/zobject3d.cpp
    ${GuiDir}/zstring.cpp ${GuiDir}/zargumentprocessor.cpp
    ${GuiDir}/zkmeansclustering.cpp ${GuiDir}/zdoublevector.cpp
    ${GuiDir}/zrandomgenerator.cpp)

  foreach (file ${ExternalSource})
    ADD_CUSTOM_COMMAND(OUTPUT ${file} COMMAND touch ${file})
  endforeach(file)

  LINK_DIRECTORIES (${BUILDEM_DIR}/lib)
  INCLUDE_DIRECTORIES(${BUILDEM_DIR}/include ${BUILDEM_DIR}/include/neurolabi
    ${GuiDir})

  ADD_EXECUTABLE (Tbar_apply_sa Tbar_apply_sa.cpp ${ExternalSource})


  SET(hdf5Lib hdf5)
  SET(hdf5LtLib hdf5_hl)
  SET(neuLib neurolabi_min)

  #FIND_LIBRARY(hdf5Lib NAMES hdf5 PATHS ${BUILDEM_DIR}/lib NO_DEFAULT_PATH)
  #FIND_LIBRARY(hdf5LtLib NAMES hdf5_hl PATHS ${BUILDEM_DIR}/lib NO_DEFAULT_PATH)
  #FIND_LIBRARY(neuLib NAMES neurolabi_min PATHS ${BUILDEM_DIR}/lib NO_DEFAULT_PATH)

  TARGET_LINK_LIBRARIES(Tbar_apply_sa ${neuLib} ${hdf5Lib} ${hdf5LtLib})

  MESSAGE(STATUS "Library path " ${CMAKE_LIBRARY_PATH} ${CMAKE_INCLUDE_PATH})
  MESSAGE(STATUS "hdf5Lib " ${hdf5Lib})
  MESSAGE(STATUS "hdf5LtLib " ${hdf5LtLib})
  MESSAGE(STATUS "neuLib " ${neuLib})
endif(BUILDEM_READY)



