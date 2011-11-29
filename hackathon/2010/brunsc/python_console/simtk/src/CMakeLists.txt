# message("simtk common")
add_definitions(-DSimTK_SimTKCOMMON_BUILDING_STATIC_LIBRARY)
file(GLOB SimTKcommon_SRCS "*.cpp")
add_library(SimTKcommon STATIC ${SimTKcommon_SRCS})

# 64 bit AMD Linux requires -fPIC flag for linking static libraries --CMB
if(UNIX AND NOT APPLE)
  if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
    set_target_properties(SimTKcommon PROPERTIES COMPILE_FLAGS -fPIC)
  endif()
endif()

