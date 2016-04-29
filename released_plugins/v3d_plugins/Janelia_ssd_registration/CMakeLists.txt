# Needs debugging on Windows
if(NOT WIN32)

if(NOT Qt5Core_FOUND)
    QT4_WRAP_UI(PLUGIN_UI_SRCS
        paradialog_rigidaffine.ui
    )
else()
    QT5_WRAP_UI(PLUGIN_UI_SRCS
        paradialog_rigidaffine.ui
    )
endif()

set(PLUGIN_HEADERS  # Q_OBJECT macro containing files only
    plugin_image_registration.h
    q_paradialog_rigidaffine.h
)
set(PLUGIN_DIRECTORY_NAME image_registration/SSD_registration)
include_directories(
    "${CMAKE_CURRENT_BINARY_DIR}" # to find generated ui_paradialog_rigidaffine.h header
    "${TIFF_INCLUDE_DIR}"
    "${CMAKE_SOURCE_DIR}/v3d_main/jba/newmat11"
)
set(PLUGIN_SOURCES
    ${PLUGIN_UI_SRCS}
    common/q_imresize.cpp
    common/q_convolve.cpp
    histogram_matching/q_histogram_matching.cpp
    q_paradialog_rigidaffine.cpp
    q_registration_common.cpp
    q_rigidaffine_registration.cpp
    plugin_image_registration.cpp
)
configure_v3d_plugin(plugin_PQ_imagereg)
target_link_libraries(plugin_PQ_imagereg
    ${TIFF_LIBRARY}
    newmat11
)

endif(NOT WIN32)
