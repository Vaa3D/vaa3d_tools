
TEMPLATE	= lib
CONFIG	+= qt plugin warn_off
#CONFIG	+= x86_64
#unix {V3DMAINPATH = ../../../v3d_main
#}
#win32{V3DMAINPATH = ..\\..\\..\\v3d_main
#}
unix {V3DMAINPATH = ../../../v3d_main
}
win32{V3DMAINPATH =  ../../../../v3d_external/v3d_main
}
INCLUDEPATH	+= $$V3DMAINPATH/basic_c_fun
INCLUDEPATH += $$V3DMAINPATH/common_lib/include
QT += widgets
HEADERS	+= movieConverter_plugin.h
SOURCES	+= movieConverter_plugin.cpp
HEADERS	+= $$V3DMAINPATH/basic_c_fun/v3d_message.h
SOURCES	+= $$V3DMAINPATH/basic_c_fun/v3d_message.cpp

TARGET	= $$qtLibraryTarget(movie_format_convert)
DESTDIR	= $$V3DMAINPATH/../bin/plugins/movies_and_snapshots/Movie_Converter

#macx{
#    QMAKE_POST_LINK = cp ./bin/mac_ffmpeg $$V3DMAINPATH/../bin/.
#}

#unix:!macx {
#    QMAKE_POST_LINK = cp ./bin/linux_ffmpeg $$V3DMAINPATH/../bin/.
#}

#win32{
#    QMAKE_POST_LINK = copy .\\bin\\windows_ffmpeg.exe $$V3DMAINPATH\\..\\bin\\.
#}

