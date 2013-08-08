
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH += $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH += $$VAA3DPATH/v3d_main/common_lib/include

HEADERS	+= extractZSlices_plugin.h
SOURCES	+= extractZSlices_plugin.cpp

HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/basic_memory.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/mg_utilities.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/mg_image_lib.h
HEADERS += $$VAA3DPATH/v3d_main/basic_c_fun/stackutil.h

SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage_create.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_4dimage.cpp

SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/basic_memory.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/mg_utilities.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/mg_image_lib.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/stackutil.cpp
SOURCES += $$VAA3DPATH/v3d_main/basic_c_fun/imageio_mylib.cpp

macx{
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib_mac64 -lv3dtiff
#    CONFIG += x86_64
}

win32{
}

unix:!macx {
    #LIBS += -L$$V3DMAINPATH/common_lib/lib -lv3dtiff
    LIBS += -L$$VAA3DPATH/v3d_main/common_lib/lib -ltiff
}

unix:LIBS += -L$$VAA3DPATH/v3d_main/common_lib/src_packages/mylib_tiff -lmylib


TARGET	= $$qtLibraryTarget(extractZSlices)
DESTDIR	= $$VAA3DPATH/bin/plugins/image_geometry/extract_Z_Slices/
