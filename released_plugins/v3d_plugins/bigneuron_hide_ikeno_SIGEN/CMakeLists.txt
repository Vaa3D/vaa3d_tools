# set(CMAKE_VERBOSE_MAKEFILE 1)
cmake_minimum_required(VERSION 2.8) # same as vaa3d
project(Vaa3D-SIGEN)
enable_testing()

option(BUILD_MAIN "Build runner program." OFF)

if(BUILD_MAIN)
  find_package(PkgConfig)
  pkg_check_modules(OPENCV REQUIRED opencv)
endif()

include_directories("${Boost_INCLUDE_DIR}")

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG --coverage")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g --coverage")
endif()

# http://stackoverflow.com/a/10055571
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++03 -Wall -Wextra -Wshadow -Wc++11-extensions -pedantic -Wno-c++11-long-long -Wno-variadic-macros")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address --coverage")
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g -fsanitize=address --coverage")
endif()

add_subdirectory(src)
add_subdirectory(test)
