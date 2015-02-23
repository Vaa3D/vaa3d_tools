# find all headers and sources
file(GLOB headers *.h)
file(GLOB sources *.cpp)

# add parent dir to include path
include_directories (../../)

# get current directory name
get_filename_component(dirname ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# add static library
add_library(ioplugin_${dirname} STATIC ${headers} ${sources})