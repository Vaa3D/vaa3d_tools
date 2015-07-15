TEMPLATE    = lib
CONFIG  += debug qt plugin warn_off
#CONFIG += x86_64

VAA3DPATH = ../../../../v3d_external
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun 
INCLUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include

INCLUDEPATH += ./include/opencv
INCLUDEPATH += ./include


LIBS += -L. -lv3dtiff -L$$VAA3DPATH/v3d_main/common_lib/lib

LIBS += -L"$$_PRO_FILE_PWD_/lib_centos"

LIBS+= -lopencv_core -lopencv_imgproc -lopencv_ml  \

HEADERS =  LCM_boost_plugin.h
HEADERS += LCM_boost_func.h
HEADERS += vn_imgpreprocess.h
HEADERS += fastmarching_tree.h
HEADERS += hierarchy_prune.h
HEADERS += fastmarching_dt.h
HEADERS += my_surf_objs.h
HEADERS += swc_to_maskimage.h

SOURCES = LCM_boost_plugin.cpp
SOURCES += LCM_boost_func.cpp
SOURCES += my_surf_objs.cpp
SOURCES += vn_imgpreprocess.cpp
SOURCES += swc_to_maskimage.cpp

HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_memory.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/stackutil.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/mg_utilities.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/mg_image_lib.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.h

# put all vaa3d related cpp file in the end !!!

SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_memory.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/stackutil.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_surf_objs.cpp

TARGET  = $$qtLibraryTarget(LCM_boost)
DESTDIR = $$VAA3DPATH/bin/plugins/LCM_boost/

