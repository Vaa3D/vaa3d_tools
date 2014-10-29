

TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
INCLUDEPATH += $$V3DMAINPATH/jba/newmat11

macx{
    LIBS += -L$$V3DMAINPATH/common_lib/lib_mac64 -lv3dtiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
#    CONFIG += x86_64
}

win32{
    LIBS += -L$$V3DMAINPATH/common_lib/winlib64 -llibtiff
    LIBS += -L$$V3DMAINPATH/common_lib/winlib64 -llibnewmat
}

unix:!macx {
    LIBS += -L$$V3DMAINPATH/common_lib/lib -ltiff
    LIBS += -L$$V3DMAINPATH/jba/c++ -lv3dnewmat
    QMAKE_CXXFLAGS += -fopenmp
    LIBS += -fopenmp
}


INCLUDEPATH += main

HEADERS += $$V3DMAINPATH/basic_c_fun/basic_memory.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_utilities.h
HEADERS += $$V3DMAINPATH/basic_c_fun/mg_image_lib.h
HEADERS += $$V3DMAINPATH/basic_c_fun/stackutil.h
HEADERS	+= MultisclaeEnhancement_plugin.h

SOURCES	+= MultisclaeEnhancement_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_utilities.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/mg_image_lib.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/stackutil.cpp
SOURCES += $$V3DMAINPATH/basic_c_fun/basic_surf_objs.cpp
SOURCES	+=  ../../../hackathon/zhi/APP2_large_scale/readrawfile_func.cpp


SOURCES += ../neurontracing_vn2/app2/my_surf_objs.cpp


TARGET	= $$qtLibraryTarget(multiscaleEnhancement)
DESTDIR	= ../../../bin/plugins/multiscaleEnhancement
