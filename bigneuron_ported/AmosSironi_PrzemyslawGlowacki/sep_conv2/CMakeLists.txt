

project(sep_conv_test1) 

find_package(ITK REQUIRED)

include(${ITK_USE_FILE})

add_executable(sep_conv_test1 MACOSX_BUNDLE sep_conv_test1.cpp)

target_link_libraries(sep_conv_test1 ${ITK_LIBRARIES})