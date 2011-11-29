TEMPLATE      = lib
CONFIG       += plugin warning_off
#CONFIG	     += X86_64 
INCLUDEPATH  += ../../../v3d_main/basic_c_fun
QMAKE_CXXFLAGS += -fpermissive 

HEADERS       = partial_align_plugin.h
HEADERS      += myfeature.h
HEADERS      += partial_align_gui.h
HEADERS      += compute_moments.h
HEADERS      += edge_detection.h
HEADERS      += ../../../v3d_main/basic_c_fun/v3d_interface.h
HEADERS      += ../../../v3d_main/basic_c_fun/img_definition.h
HEADERS      += ../../../v3d_main/basic_c_fun/v3d_basicdatatype.h

SOURCES       = partial_align_plugin.cpp
SOURCES      += ../../../v3d_main/basic_c_fun/v3d_message.cpp

TARGET        = $$qtLibraryTarget(partial_align)
DESTDIR       = ~/Applications/v3d/plugins/partial_align/

