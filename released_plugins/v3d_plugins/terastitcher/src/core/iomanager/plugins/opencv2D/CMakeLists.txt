# find all headers and sources
file(GLOB headers *.h)
file(GLOB sources *.cpp)

# add parent dir to include path
include_directories (../../)

# get current directory name
get_filename_component(dirname ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# add static library
add_library(ioplugin_${dirname} STATIC ${headers} ${sources})

# set up OpenCV library
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${OpenCV_LIB_DIR})

# link 3rd party libraries
target_link_libraries(ioplugin_${dirname} ${OpenCV_LIBS})