TEMPLATE    = lib
CONFIG  += debug qt plugin warn_off
#CONFIG += x86_64

VAA3DPATH = /home/gulin/v3d_external
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun 
INClUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include

INCLUDEPATH += ./include/opencv

LIBS += -L. -lv3dtiff -L$$VAA3DPATH/v3d_main/common_lib/lib

LIBS += -L"$$_PRO_FILE_PWD_/lib"

LIBS+= -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml  \

HEADERS =  Opencv_example_plugin.h
HEADERS += Opencv_example_func.h

SOURCES = Opencv_example_plugin.cpp
SOURCES += Opencv_example_func.cpp

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

TARGET  = $$qtLibraryTarget(Opencv_example)
DESTDIR = /home/gulin/vaa3d/plugins/Opencv_example/
