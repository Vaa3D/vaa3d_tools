
set(PLUGIN_HEADERS mapviewer.h)
set(PLUGIN_SOURCES 
    mg_image_lib11.cpp
    stackutil-11.cpp
    mapviewer.cpp
    ../../../v3d_main/basic_c_fun/mg_utilities.cpp
    ../../../v3d_main/basic_c_fun/v3d_message.cpp
)
set(PLUGIN_DIRECTORY_NAME image_stitching/Map_View)

include_directories(../istitch)
include_directories(${TIFF_INCLUDE_DIR})
include_directories(${QT4_DEMOS_INCLUDE_DIR})
include_directories(${FFTW_INCLUDE_DIR})
if(NOT MSVC)
  configure_v3d_plugin(mapview)
  target_link_libraries(mapview ${TIFF_LIBRARY})
endif()

# TEMPLATE      = lib
# CONFIG       += qt plugin warn_off
# #CONFIG       += x86_64
# #CONFIG       +=release
# 
# #include(/usr/local/Trolltech/Qt-4.5.2/demos/shared/shared.pri)
# 
# V3DMAINDIR = ../../../v3d_main
# QT_DIR = $$dirname(QMAKE_QMAKE)/..
# 
# INCLUDEPATH  += $$V3DMAINDIR/basic_c_fun
# INCLUDEPATH  += $$V3DMAINDIR/common_lib/include
# INCLUDEPATH  += ../istitch
# INCLUDEPATH  += $$QT_DIR/demos/shared/
# #HEADERS      +=/usr/local/Trolltech/Qt-4.5.2/demos/shared/arthurwidgets.h
# HEADERS      +=colormap.h
# 
# HEADERS      += mg_image_lib11.h
# HEADERS      += stackutil-11.h
# HEADERS      += ../istitch/y_imglib.h
# 
# HEADERS      += mapviewer.h \
# 		$$V3DMAINDIR/basic_c_fun/mg_utilities.h 
# 
# SOURCES       += mg_image_lib11.cpp 
# SOURCES       += stackutil-11.cpp
# SOURCES       += mapviewer.cpp \
# 		$$V3DMAINDIR/basic_c_fun/mg_utilities.cpp 
# SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp
# 
# LIBS     += -lm -lpthread
# LIBS	    += -L$$V3DMAINDIR/common_lib/lib -lv3dtiff
#         += -L$$QTDIR/demos/shared/
# #LIBS += -framework CoreServices
# TARGET        = $$qtLibraryTarget(mapviewer)
# DESTDIR       = ../../v3d/plugins/image_stitching/Map_View
# 
