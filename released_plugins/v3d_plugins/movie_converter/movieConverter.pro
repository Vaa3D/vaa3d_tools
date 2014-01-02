
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
DESTDIR	= ../../v3d/plugins/movies/Movie_Converter

macx{
    QMAKE_POST_LINK = cp ./bin/mac_ffmpeg ../../v3d/.
}

unix:!macx {
    QMAKE_POST_LINK = cp ./bin/linux_ffmpeg ../../v3d/.
}

win32{
    QMAKE_POST_LINK = copy .\\bin\\windows_ffmpeg.exe ..\\..\\v3d\\.
}

