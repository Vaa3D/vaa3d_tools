set(names
  elf32lsb.bin
  elf32msb.bin
  elf64lsb.bin
  elf64msb.bin
  )

# Prepare binaries on which to operate.
set(in "/local1/vaa3d_test/vaa3d_tools/hackathon/MK/NeuronStructNavigator/cmake-3.6.2/Tests/CMakeTests/ELF")
set(out "/local1/vaa3d_test/vaa3d_tools/hackathon/MK/NeuronStructNavigator/cmake-3.6.2/Tests/CMakeTests/ELF-Out")
file(REMOVE_RECURSE "${out}")
file(MAKE_DIRECTORY "${out}")
foreach(f ${names})
  file(COPY ${in}/${f} DESTINATION ${out} NO_SOURCE_PERMISSIONS)
  list(APPEND files "${out}/${f}")
endforeach()

foreach(f ${files})
  # Check for the initial RPATH.
  file(RPATH_CHECK FILE "${f}" RPATH "/sample/rpath")
  if(NOT EXISTS "${f}")
    message(FATAL_ERROR "RPATH_CHECK removed ${f}")
  endif()

  # Change the RPATH.
  file(RPATH_CHANGE FILE "${f}"
    OLD_RPATH "/sample/rpath"
    NEW_RPATH "/rpath/sample")
  set(rpath)
  file(STRINGS "${f}" rpath REGEX "/rpath/sample" LIMIT_COUNT 1)
  if(NOT rpath)
    message(FATAL_ERROR "RPATH not changed in ${f}")
  endif()

  # Remove the RPATH.
  file(RPATH_REMOVE FILE "${f}")
  set(rpath)
  file(STRINGS "${f}" rpath REGEX "/rpath/sample" LIMIT_COUNT 1)
  if(rpath)
    message(FATAL_ERROR "RPATH not removed from ${f}")
  endif()

  # Check again...this should remove the file.
  file(RPATH_CHECK FILE "${f}" RPATH "/sample/rpath")
  if(EXISTS "${f}")
    message(FATAL_ERROR "RPATH_CHECK did not remove ${f}")
  endif()
endforeach()
