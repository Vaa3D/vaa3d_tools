# CMakeLists.txt in common dir

include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)

# activate Qt-based code
if(WITH_GUI)
	add_definitions(-DWITH_QT)	
endif(WITH_GUI)

# add executable from the all .h and .cpp files
file(GLOB tc_headers *.h)
file(GLOB tc_sources *.cpp)

# set CMake 3.* automoc on, so that CMake automatically determines the source files that need moc pre-processing
set(CMAKE_AUTOMOC ON)

# check for either Qt5 or Qt4
if(WITH_GUI)
	option (WITH_QT5 "With QT5" OFF) 
	if(WITH_QT5)
		find_package(Qt5Widgets REQUIRED)
		add_library(common ${tc_headers} ${tc_sources})
		target_link_libraries(common Qt5::Widgets)
	endif(WITH_QT5)
	option (WITH_QT4 "With QT4" OFF) 
	if(WITH_QT4)
		find_package(Qt4 REQUIRED)
		add_library(common ${tc_headers} ${tc_sources})
		target_link_libraries(common Qt4::QtGui)
	endif(WITH_QT4)
	if(NOT WITH_QT4 AND NOT WITH_QT5)
		message(FATAL_ERROR "Please select at least one Qt library (Qt4 or Qt5)")
	endif(NOT WITH_QT4 AND NOT WITH_QT5)
else(WITH_GUI)
	add_library(common ${tc_headers} ${tc_sources})
endif(WITH_GUI)
set_target_properties(common PROPERTIES POSITION_INDEPENDENT_CODE ON)

