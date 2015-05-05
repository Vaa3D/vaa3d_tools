cmake_minimum_required(VERSION 2.8)

project(sep_conv_test2)

# Find ITK.
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

#FIND_PACKAGE(VTK REQUIRED)
#INCLUDE(${VTK_USE_FILE})

#FIND_PACKAGE(OpenMP)
#IF(OPENMP_FOUND)
#message("OpenMP FOUND")
#  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
#  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}" )
  # Currently FindOpenMP fails and doesn't give _any_ linker flags
#  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fopenmp -lgomp")
#  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -fopenmp -lgomp")
#ELSE(OPENMP_FOUND)
#  MESSAGE(FATAL_ERROR "Cannot build without OpenMP.")
#endif(OPENMP_FOUND)

#if( "${ITK_VERSION_MAJOR}" LESS 4 )
#  target_link_libraries(readSeriesWrite ITKReview ${ITK_LIBRARIES})
#else( "${ITK_VERSION_MAJOR}" LESS 4 )
#  target_link_libraries(readSeriesWrite ${ITK_LIBRARIES})
#endif( "${ITK_VERSION_MAJOR}" LESS 4 )

#find_package(ItkVtkGlue REQUIRED)
#include(${ItkVtkGlue_USE_FILE})


add_executable(sep_conv_test2 sep_conv_test2.cpp)

target_link_libraries(sep_conv_test2 ${ITK_LIBRARIES})

