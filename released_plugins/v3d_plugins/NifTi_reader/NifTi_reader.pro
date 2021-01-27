TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
V3DMAINPATH = ../../../../v3d_external
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/common_lib/include
INCLUDEPATH	+= $$V3DMAINPATH/v3d_main/v3d
INCLUDEPATH	+= ./include
macx{
    LIBS += ./lib/mac/libz.a    
    LIBS += ./lib/mac/libniftiio.a
    LIBS += ./lib/mac/libznz.a
    LIBS += ./lib/mac/libnifticdf.a
}

win32{
    LIBS += ./lib/win32/zlib.lib    
    LIBS += ./lib/win32/nifticdf.lib
    LIBS += ./lib/win32/niftiio.lib
    LIBS += ./lib/win32/znz.lib
}

unix:!macx {
    LIBS += ./lib/unix/libz.a
    LIBS += ./lib/unix/libniftiio.a
    LIBS += ./lib/unix/libznz.a
    LIBS += ./lib/unix/libnifticdf.a
}


HEADERS	+= NifTi_reader_plugin.h
SOURCES	+= NifTi_reader_plugin.cpp
SOURCES	+= $$V3DMAINPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(NifTi_reader)
DESTDIR	= $$V3DMAINPATH/bin/plugins/data_IO/NIFTI_file_reader
