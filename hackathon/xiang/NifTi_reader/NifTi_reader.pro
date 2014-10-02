TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
VAA3DPATH = ../../../../v3d_external
INCLUDEPATH	+= $$VAA3DPATH/v3d_main/basic_c_fun
INCLUDEPATH	+= ./include
macx{
    LIBS += ./lib/mac/libz.a    
    LIBS += ./lib/mac/libfslio.a
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
    LIBS += ./lib/unix/libfslio.a
    LIBS += ./lib/unix/libniftiio.so.2.0.0
    LIBS += ./lib/unix/libznz.so.2.0.0
    LIBS += ./lib/unix/libnifticdf.so.2.0.0
}


HEADERS	+= NifTi_reader_plugin.h
SOURCES	+= NifTi_reader_plugin.cpp
SOURCES	+= $$VAA3DPATH/v3d_main/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(NifTi_reader)
DESTDIR	= ./release