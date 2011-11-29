cmake_minimum_required(VERSION 2.6)

set(PLUGIN_DIRECTORY_NAME Movie_Maker)
set(PLUGIN_SOURCES MovieMaker.cpp)
set(PLUGIN_HEADERS MovieMaker.h)

# To support local test building, conditionally set certain variables
if(NOT FinishedPlugins)
    add_custom_target(FinishedPlugins)
endif()
if(NOT INSTALLATION_DIRECTORY)
    set(INSTALLATION_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif()
include(../../../released_plugins/CMake/PluginConfiguration.cmake)

# Qt is needed for GUI window.
find_package(Qt4 REQUIRED)
include( ${QT_USE_FILE} )

QT4_WRAP_UI(QT_UI_SRCS
  movie_maker_dialog.ui
)

QT4_WRAP_CPP(QT_MOC_SRCS
#   MovieMaker.h
)

# Qt dynamically generates some headers here
include_directories(${CMAKE_CURRENT_BINARY_DIR})

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../v3d_main/basic_c_fun/"
    )

configure_v3d_plugin(MovieMaker)

target_link_libraries(MovieMaker
    ${QT_QTGUI_LIBRARY}
)

find_path(PLUGINS_PARENT_PATH plugins
    PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../build_v3d_cmake_eclipse/v3d"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../build_cmake/v3d"
    PATH_SUFFIXES
        "Mac_Fat/v3d.app/Contents/MacOS"
        "Linux_32"
        "Linux_64"
    NO_DEFAULT_PATH
    )
    
if(PLUGINS_PARENT_PATH)
    set(PLUGINS_PATH "${PLUGINS_PARENT_PATH}/plugins")
else()
    set(PLUGINS_PATH "${CMAKE_CURRENT_BINARY_DIR}/plugins")
endif()

set(PLUGIN_OUTPUT_DIR
    "${PLUGINS_PATH}/${PLUGIN_DIRECTORY_NAME}"
    CACHE PATH "Where to build Movie Maker plugin")

set_target_properties(MovieMaker PROPERTIES 
    ARCHIVE_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}"
)

