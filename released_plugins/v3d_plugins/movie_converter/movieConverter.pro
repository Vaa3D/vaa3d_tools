
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
V3DMAINPATH = ../../../v3d_main
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include

HEADERS	+= movieConverter_plugin.h
SOURCES	+= movieConverter_plugin.cpp
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(movie_format_convert)
DESTDIR	= ../../../bin/plugins/movies/Movie_Converter

macx{

    QMAKE_POST_LINK = cp ./bin/mac_ffmpeg ../../../bin

}

win32{
}

unix:!macx {

    QMAKE_POST_LINK = cp ./bin/linux_ffmpeg ../../../bin

}
