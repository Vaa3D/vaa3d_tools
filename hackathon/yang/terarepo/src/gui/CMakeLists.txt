# add used modules to include path
include_directories (${TeraStitcher_SOURCE_DIR}/imagemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/iomanager)
include_directories (${TeraStitcher_SOURCE_DIR}/stitcher)
include_directories (${TeraStitcher_SOURCE_DIR}/utils/volumeconverter)
include_directories (${TeraStitcher_SOURCE_DIR}/volumemanager)
include_directories (${TeraStitcher_SOURCE_DIR}/crossmips)
include_directories (${TeraStitcher_SOURCE_DIR}/common)


# add 3rd party modules to include path
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty)
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty/tclap)
include_directories(${TeraStitcher_SOURCE_DIR}/3rdparty/tinyxml)


# collect source files
file(GLOB terastitcher_gui_sources *.h *.cpp *.qrc)


# enable CMake 3.* Meta-Object Compiler (MOC) and Resource Collection Compiler (RCC) 
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


# create option to link Qt4 or Qt5 (default)
option (WITH_QT5 "With Qt5" ON) 
option (WITH_QT4 "With Qt4" OFF) 


# find the Qt components we need and store them in qt_lib_files
# so that we can copy the libs in the installation folder (linux)
if(WITH_QT5)
	find_package(Qt5Widgets REQUIRED)
	get_target_property(tmp Qt5::Widgets LOCATION)
	LIST(APPEND qt_lib_files ${tmp})
	get_target_property(tmp Qt5::Core LOCATION)
	LIST(APPEND qt_lib_files ${tmp})
	get_target_property(tmp Qt5::Gui LOCATION)
	LIST(APPEND qt_lib_files ${tmp})
endif()
if(WITH_QT4)
	find_package(Qt4 REQUIRED)
	get_target_property(tmp Qt4::QtGui LOCATION)
	LIST(APPEND qt_lib_files ${tmp})
	get_target_property(tmp Qt4::QtCore LOCATION)
	LIST(APPEND qt_lib_files ${tmp})
endif()
if(NOT WITH_QT4 AND NOT WITH_QT5)
	message(FATAL_ERROR "Please select at least one Qt library (Qt4 or Qt5)")
endif()



# set platform-specific bundle icons
if(APPLE)
  list(APPEND terastitcher_gui_sources icons/terastitcher.icns)
  set_source_files_properties(icons/terastitcher.icns PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
#elseif(UNIX)
  #install(FILES "icons/terastitcher.desktop" DESTINATION "${INSTALL_XDG_APPS_DIR}")
  #install(FILES "icons/terastitcher.png" DESTINATION "${INSTALL_XDG_ICON_DIR}")
elseif(WIN32)
  list(APPEND terastitcher_gui_sources icons/terastitcher.rc)
endif()


# add executable and link Qt components
add_executable(terastitcher-gui WIN32 MACOSX_BUNDLE ${terastitcher_gui_sources})
if(WITH_QT5)
	target_link_libraries(terastitcher-gui Qt5::Widgets)
endif()
if(WITH_QT4)
	target_link_libraries(terastitcher-gui Qt4::QtGui)
endif()

# link to internal dependencies
target_link_libraries(terastitcher-gui volumeconverter )
target_link_libraries(terastitcher-gui imagemanager )
target_link_libraries(terastitcher-gui iomanager )
target_link_libraries(terastitcher-gui volumemanager )
target_link_libraries(terastitcher-gui tinyxml )
target_link_libraries(terastitcher-gui stitcher )
target_link_libraries(terastitcher-gui crossmips )
target_link_libraries(terastitcher-gui common )

# installation
install(TARGETS terastitcher-gui
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  BUNDLE DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

# make sure bundle is standalone and it can be drag-and-dropped
if(APPLE)
	set_target_properties(terastitcher-gui PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/osinstall/Info.plist)
endif()
if((APPLE OR WIN32))
	set(pfx ${CMAKE_INSTALL_BINDIR}/)
	set(sfx "")
	if(APPLE)
	set(sfx ".app")
	elseif(WIN32)
	set(sfx ".exe")
	endif()

	get_target_property(output_name terastitcher-gui NAME)
	set(exe "${pfx}${output_name}${sfx}")

	include(InstallRequiredSystemLibraries)
	if(WITH_QT4)
		include(DeployQt4)
		install_qt4_executable(${exe})
	elseif(WITH_QT5)
		include(DeployQt5)
		install_qt5_executable(${exe})
	endif()
endif()

# DeployQt5 does not support Linux
# so we just copy the (symlink-resolved) Qt libs in the installation folder
if(UNIX AND NOT APPLE)
	FOREACH(qt_lib_file ${qt_lib_files})
		get_filename_component(qt_lib_real_file ${qt_lib_file} REALPATH)
		get_filename_component(qt_lib_fname "${qt_lib_file}" NAME)
		if(WITH_QT4)
			set (qt_lib_fname ${qt_lib_fname}.4)
		elseif(WITH_QT5)
			set (qt_lib_fname ${qt_lib_fname}.5)
		endif()
		install(FILES ${qt_lib_real_file} DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME ${qt_lib_fname})
		message(STATUS "Qt lib " ${qt_lib_real_file} " will be installed to " ${CMAKE_INSTALL_BINDIR} " and renamed to " ${qt_lib_fname})
	ENDFOREACH()
	# also copy a launch script that sets LD_LIBRARY_PATH to local path
	install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/osinstall/start_terastitcher.sh DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
