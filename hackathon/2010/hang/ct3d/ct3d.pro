
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
INCLUDEPATH	+= ../../../v3d_main/basic_c_fun

LIBS += -ltiff -llpsolve55

#DEFINES += __v3d__

HEADERS	= ct3d_plugin.h
HEADERS	+= ct3d_func.h
HEADERS	+= ct3d_gui.h
HEADERS += v3d_monitor.h

HEADERS += coseg/component_tree.h
HEADERS += coseg/myalgorithms.h
HEADERS += coseg/CT3D/cell_track.h
HEADERS += coseg/CT3D/palette.h
HEADERS += coseg/CT3D/bipartite.h
HEADERS += coseg/CT3D/cell_track_controller.h
HEADERS += coseg/AT3D/widgets/cellwidget.h

SOURCES	= ct3d_plugin.cpp
SOURCES	+= ct3d_func.cpp
SOURCES	+= ../../../v3d_main/basic_c_fun/v3d_message.cpp

SOURCES += coseg/component_tree.cpp
SOURCES += coseg/myalgorithms.cpp
SOURCES += coseg/CT3D/cell_track.cpp
SOURCES += coseg/CT3D/palette.cpp
SOURCES += coseg/CT3D/bipartite.cpp
SOURCES += coseg/CT3D/cell_track_controller.cpp
SOURCES += coseg/AT3D/widgets/cellwidget.cpp

TARGET	= $$qtLibraryTarget(ct3d)
DESTDIR	= ~/Applications/v3d/plugins/ct3d/
