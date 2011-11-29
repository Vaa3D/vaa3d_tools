cmake_minimum_required(VERSION 2.6)

# Default to building fat binaries, even on Leopard
if(APPLE)
    if(NOT CMAKE_OSX_ARCHITECTURES)
         set(CMAKE_OSX_ARCHITECTURES "i386;x86_64" CACHE STRING "" FORCE)
    endif()
endif()

# Qt is needed for GUI window.
find_package(Qt4 REQUIRED)
include( ${QT_USE_FILE} )

QT4_WRAP_UI(QT_UI_PYTHON_EDITOR_SRCS
    python_editor.ui
)

QT4_WRAP_CPP(QT_MOC_PYTHON_EDITOR_SRCS
    PythonEditorWidget.h
    PythonSyntaxHighlighter.h
)

QT4_ADD_RESOURCES(QT_RCC_SRCS python_editor.qrc)

# Qt dynamically generates some headers here
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(V3D_PYTHON_EDITOR_SRCS
    PythonEditorWidget.h
    PythonSyntaxHighlighter.h
    PythonEditorWidget.cpp
    PythonSyntaxHighlighter.cpp
    ${QT_UI_PYTHON_EDITOR_SRCS}
    ${QT_MOC_PYTHON_EDITOR_SRCS}
)

#################################################################
# testPythonEditor is a test program for use during development #
#################################################################

add_executable(testPythonEditor
    testPythonEditor.cpp
    ${V3D_PYTHON_EDITOR_SRCS}
)

target_link_libraries(testPythonEditor
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
)

